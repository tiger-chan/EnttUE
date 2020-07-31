#pragma once

#include "CoreMinimal.h"
#include "task.hpp"
#include "task_registrar.hpp"

namespace tc
{
template <typename... task_args_t> class task_builder {
	using registar_t = task_registar<task_args_t...>;
	using graph_t = task_graph<task_args_t...>;
	using task_t = typename graph_t::task_t;

    public:
	task_builder(registar_t *owner_, FName name, uint32 priority)
		: owner{ owner_ }, graph{ MakeShared<graph_t>(name, priority) }
	{
	}

	template <typename func_t>
	void add(task_dependency &&dependencies, func_t &&func)
	{
		auto task = MakeShared<task_t>();

		task->owner = graph;
		task->dependencies = std::move(dependencies);
		task->work = std::move(func);

		graph->append(task);
	}

	registar_t &finish_graph()
	{
		owner->add_graph(std::move(graph));
		return *owner;
	}

    private:
	registar_t *owner = nullptr;
	TSharedPtr<graph_t> graph = nullptr;
};

} // namespace tc
