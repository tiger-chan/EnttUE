#pragma once

#include "CoreMinimal.h"
#include "task.hpp"
#include "task_builder.hpp"
#include "task_registrar.hpp"

namespace tc
{
template <typename... task_args_t>
class task_scheduler : task_registar<task_args_t...> {
	using task_builder_t = task_builder<task_args_t...>;
	using graph_t = task_graph<task_args_t...>;
	using task_t = task<task_args_t...>;

	friend class task_builder_t;

    protected:
	struct task_handle {
		uint32_t task_index = 0;
		TSharedPtr<task_t> task = nullptr;
	};

	struct vertex {
		int32_t task_id;
		TSet<int32_t> incoming;
		TSet<int32_t> outgoing;
	};

	struct edge {
		TSet<int32_t> read;
		TSet<int32_t> write;
	};

    public:
	virtual ~task_scheduler() = default;
	void execute(task_args_t &&... args)
	{
		if (is_running) {
			return;
		}
		is_running = true;
		sort();

		execute_intern(std::forward<task_args_t &&>(args)...);
	}

	task_builder_t build_task_graph(FName name, uint32 priority)
	{
		return task_builder_t{ this, name, priority };
	}

    protected:
	virtual void execute_intern(task_args_t &&... args)
	{
		for (TArray<task_handle> &groups : ordered_tasks) {
			for (task_handle &handle : groups) {
				task_t &task = *handle.task;
				if constexpr (sizeof...(args) == 0) {
					task();
				} else {
					task(std::forward<task_args_t>(
						args)...);
				}
			}
		}

		set_complete();
	}

	virtual TArray<TArray<task_handle>>
	sort_intern(TMap<uint32_t, edge> &edges, TArray<vertex> &vertices,
		    TArray<task_handle> &tasks)
	{
		// sort by incoming edge count
		auto vertex_sort = [](const vertex &task_1,
				      const vertex &task_2) {
			if (task_1.incoming.Num() < task_2.incoming.Num()) {
				return true;
			} else if (task_1.incoming.Num() ==
				   task_2.incoming.Num()) {
				return task_1.outgoing.Num() >
				       task_2.outgoing.Num();
			}
			return false;
		};

		TArray<vertex *> vert_stack;
		vert_stack.Reserve(vertices.Num());
		for (vertex &vert : vertices) {
			vert_stack.Emplace(&vert);
		}

		vert_stack.Heapify(vertex_sort);
		TArray<TArray<int32_t>> ordered_verts;
		ordered_verts.Reserve(vertices.Num());
		while (vert_stack.Num()) {
			TArray<int32_t> group;
			{
				vertex *vert = vert_stack.HeapTop();

				do {
					vert = vert_stack.HeapTop();
					group.Push(vert->task_id);
					vert_stack.HeapPopDiscard(vertex_sort);
				} while (vert_stack.Num() > 0 &&
					 vert_stack.HeapTop()->incoming.Num() ==
						 0);
			}

			ordered_verts.Add(group);
			for (int32_t id : group) {
				vertex &vert = vertices[id];
				for (int32_t outgoing : vert.outgoing) {
					vertex &outgoing_vert =
						vertices[outgoing];
					outgoing_vert.incoming.Remove(
						vert.task_id);
				}
			}

			vert_stack.HeapSort(vertex_sort);
		}

		TArray<TArray<task_handle>> results;
		results.Reserve(ordered_verts.Num());
		for (int32_t i = 0; i < ordered_verts.Num(); ++i) {
			auto &order_verts = ordered_verts[i];
			auto &verts = results.Emplace_GetRef();
			verts.Reserve(order_verts.Num());
			for (int32_t j = 0; j < order_verts.Num(); ++j) {
				const vertex &vert = vertices[order_verts[j]];
				task_handle &handle = tasks[vert.task_id];
				TSharedPtr<task_t> &task = handle.task;

				verts.Emplace(std::move(handle));
			}
		}

		return results;
	}

	void set_complete()
	{
		is_running = false;
	}

	TArray<TSharedPtr<graph_t>> graphs;
	TArray<TArray<task_handle>> ordered_tasks;

    private:
	void add_graph(TSharedPtr<graph_t> &&graph) override
	{
		graphs.Emplace(graph);
		is_dirty = true;
	}

	void sort()
	{
		if (!is_dirty) {
			return;
		}

		auto &&[edges, vertices, tasks] = flatten_graph();

		ordered_tasks = sort_intern(edges, vertices, tasks);

		is_dirty = false;
	}

	std::tuple<TMap<uint32_t, edge>, TArray<vertex>, TArray<task_handle>>
	flatten_graph()
	{
		TMap<uint32_t, edge> edges;
		TArray<vertex> vertices;
		TArray<task_handle> tasks;

		// Reserve enough space to begin processing
		// (conservitively, so we allocate fewer times).
		constexpr int32_t initial_edge_count = 100;
		edges.Reserve(initial_edge_count);

		// Reserve enough space such that we likely won't need to reallocate.
		// To this end we are assuming there is a mean of n number of tasks per system.
		constexpr int32_t tasks_per_system = 3;
		vertices.Reserve(graphs.Num() * tasks_per_system);
		tasks.Reserve(graphs.Num() * tasks_per_system);

		// flatten graph data for later use.
		for (TSharedPtr<graph_t> &graph : graphs) {
			uint32_t task_index = 0;
			TSharedPtr<task_t> task = graph->get_front();
			while (task.IsValid()) {
				auto index = tasks.Emplace(
					task_handle{ ++task_index, task });
				vertex &vert = vertices.Emplace_GetRef(
					vertex{ index });

				const task_dependency &task_dep =
					task->get_dependencies();

				for (const int32_t &comp :
				     task_dep.get_reads()) {
					edge &component = edges.FindOrAdd(comp);
					component.read.Add(index);
				}

				for (const int32_t &comp :
				     task_dep.get_writes()) {
					edge &component = edges.FindOrAdd(comp);
					component.write.Add(index);
				}

				task = task->get_next();
			}
		}

		// Add all of the edges
		for (TPair<uint32_t, edge> &kv_pair : edges) {
			edge e = kv_pair.Value;
			for (int32_t v_id : e.read) {
				vertex &vert = vertices[v_id];

				if (e.write.Num() == 0) {
					task_handle &handle = tasks[v_id];
					TSharedPtr<task_t> &task = handle.task;
					FName name{
						task->get_owner()->get_name()
					};

					UE_LOG(LogTemp, Warning,
					       TEXT("dependency `%d` cannot be resolved for `%s -- %d`"),
					       kv_pair.Key, *name.ToString(),
					       v_id);
				}

				for (int32_t ov_id : e.write) {
					// Skip if this edge would be to the same vertex
					if (v_id == ov_id) {
						continue;
					}

					vertex &other_vert = vertices[ov_id];
					vert.incoming.Emplace(ov_id);
					other_vert.outgoing.Emplace(v_id);
				}
			}
		}

		return std::make_tuple(std::move(edges), std::move(vertices),
				       std::move(tasks));
	}

	bool is_dirty = false;
	bool is_running = false;
};
} // namespace tc
