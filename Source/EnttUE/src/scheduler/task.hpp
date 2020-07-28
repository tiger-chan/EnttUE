#pragma once

#include "CoreMinimal.h"
#include "entt/core/type_info.hpp"

namespace tc
{
	enum class access_t
	{
		read = 1,
		write = 2,
		read_write = read | write
	};

	namespace details
	{
		template <typename... T>
		struct read_dependency
		{
		};

		template <typename... T>
		struct write_dependency
		{
		};

		template <typename... T>
		struct read_write_dependency
		{
		};
	} // namespace details

	template <typename... T>
	static constexpr details::read_dependency<T...> read_dep_list{};

	template <typename... T>
	static constexpr details::write_dependency<T...> write_dep_list{};

	template <typename... T>
	static constexpr details::read_write_dependency<T...> read_write_dep_list{};

	class task_dependency
	{
		// allocating in bulk as it's likely that we will need to add more than one
		// when adding one.
		using allocator_t = TInlineSetAllocator<4>;
		using data_t = TSet<uint32_t, DefaultKeyFuncs<uint32_t>, allocator_t>;

	public:
		task_dependency() = default;

		template <typename... read_only_t, typename... write_only_t, typename... read_write_t>
		task_dependency(
			details::read_dependency<read_only_t...> read_only,
			details::write_dependency<write_only_t...> write_only = write_dep_list<write_only_t...>,
			details::read_write_dependency<read_write_t...> read_write = read_write_dep_list<read_write_t...>)
		{
			(add<read_only_t>(access_t::read), ...);
			(add<write_only_t>(access_t::write), ...);
			(add<read_write_t>(access_t::read_write), ...);
		}

		template <typename... write_only_t, typename... read_write_t>
		task_dependency(
			details::write_dependency<write_only_t...> write_only,
			details::read_write_dependency<read_write_t...> read_write = read_write_dep_list<read_write_t...>)
			: task_dependency(read_dep_list<>, write_only, read_write) {}

		template <typename... read_write_t>
		task_dependency(
			details::read_write_dependency<read_write_t...> read_write)
			: task_dependency(read_dep_list<>, write_dep_list<>, read_write) {}

		template <typename component_t>
		void add(access_t access = access_t::read)
		{
			using ul_t = std::underlying_type_t<access_t>;
			auto a = static_cast<ul_t>(access);
			if (a & static_cast<ul_t>(access_t::read))
			{
				reads.Add(entt::type_info<component_t>::id());
			}

			if (a & static_cast<ul_t>(access_t::write))
			{
				writes.Add(entt::type_info<component_t>::id());
			}
		}

		size_t read_size() const { return reads.Num(); }
		size_t write_size() const { return writes.Num(); }

		const data_t &get_reads() const { return reads; }
		const data_t &get_writes() const { return writes; }

	private:
		data_t reads;
		data_t writes;
	};

	template <typename... task_args_t>
	class task_builder;

	template <typename... task_args_t>
	class task_graph;

	template <typename... task_args_t>
	class task
	{
		friend class task_builder<task_args_t...>;
		friend class task_graph<task_args_t...>;

		using graph_t = task_graph<task_args_t...>;

	public:
		void operator()(task_args_t &&... args) { work(std::forward<task_args_t>(args)...); }

		TSharedPtr<task<task_args_t...>> get_next() { return next; }

		const task_dependency &get_dependencies() const { return dependencies; }
		TSharedPtr<graph_t> get_owner() const { return owner.Pin(); }

	private:
		task_dependency dependencies;
		TSharedPtr<task<task_args_t...>> next = nullptr;
		TWeakPtr<graph_t> owner = nullptr;
		TFunction<void(task_args_t &&...)> work;
	};

	template <typename... task_args_t>
	class task_graph
	{
	public:
		using task_t = task<task_args_t...>;
		using task_ptr_t = TSharedPtr<task_t>;

		task_graph(FName name_, uint32 priority_) : name{name_},
													priority{priority_}
		{
		}

		void append(const task_ptr_t &task)
		{
			if (front == nullptr)
			{
				back = task;
				front = task;
			}
			else
			{
				back->next = task;
				back = task;
			}
		}

		void operator()(task_args_t &&... args)
		{
			task_ptr_t task = front;
			while (task.IsValid())
			{
				(*task)(std::forward<task_args_t>(args)...);
				task = task->next;
			}
		}

		inline void execute(task_args_t &&... args)
		{
			this->operator()(std::forward<task_args_t>(args)...);
		}

		inline task_ptr_t get_front() { return front; }

		inline FName get_name() const { return name; }

	private:
		FName name;
		uint32 priority;

		task_ptr_t front = nullptr;
		task_ptr_t back = nullptr;
	};

} // namespace tc
