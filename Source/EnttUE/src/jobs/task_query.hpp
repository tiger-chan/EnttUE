#pragma once

#include "CoreMinimal.h"
#include "core/fwd.hpp"
#include "fwd.hpp"
#include "entt/core/type_traits.hpp"
#include "entt/entity/observer.hpp"
#include "task.hpp"

namespace tc
{
template <typename Type, entt::id_type Id> struct observer_wraper {
	template <typename Collector>
	observer_wraper(ecs_registry &reg, Collector collector) : observer{ reg, collector }
	{
	}
	entt::observer observer;
};

template <typename... Args> struct observer_task : public task<ecs_registry &, Args...> {
	using work_t = TFunction<void(entt::observer &, ecs_registry &, Args &&...)>;
	using observer_request_t = TFunction<void(ecs_registry &, entt::observer *&)>;

	void run(ecs_registry &reg, Args &&... args)
	{
		entt::observer *observer = nullptr;
		observer_retrieval(reg, observer);
		work(*observer, reg, std::forward<Args>(args)...);
	}

	work_t work;
	observer_request_t observer_retrieval;
};

template <typename... Args> struct view_task : public task<ecs_registry &, Args...> {
	using work_t = TFunction<void(ecs_registry &, Args &&...)>;

	void run(ecs_registry &reg, Args &&... args)
	{
		work(reg, std::forward<Args>(args)...);
	}

	work_t work;
};

template <typename Requirements, typename System, typename Type, typename... Args>
struct job_requirements {
	job_requirements(System *system, TSharedPtr<Type> &&task) : system_{ system }, task_{ task }
	{
	}

	template <typename... Reads> Requirements &add_read() noexcept
	{
		assert(task_.IsValid());
		task_data_access &data = task_->data;
		(data.add<Reads>(access_t::read), ...);
		return *static_cast<Requirements *>(this);
	}

	template <typename... Writes> Requirements &add_write() noexcept
	{
		assert(task_.IsValid());
		task_data_access &data = task_->data;
		(data.add<Writes>(access_t::write), ...);
		return *static_cast<Requirements *>(this);
	}

	template <typename Func> void schedule(Func func, EAsyncExecution execution_method = EAsyncExecution::TaskGraph) noexcept
	{
		assert(task.IsValid());
		static_cast<Requirements *>(this)->set_work(*task_, std::forward<Func>(func));

		task_->execution_method = execution_method;
		system_->add_task(task_);
	}

	template <typename Func> void schedule_parallel(Func func, EAsyncExecution execution_method = EAsyncExecution::TaskGraph) noexcept
	{
		assert(task.IsValid());
		static_cast<Requirements *>(this)->set_work(*task_, std::forward<Func>(func));

		task_->execution_method = execution_method;
		task_->can_parallelize = true;
		system_->add_task(task_);
	}

    private:
	System *system_;
	TSharedPtr<Type> task_;
};

template <typename System, typename Type, typename... Args>
struct observer_job_requirements
	: public job_requirements<observer_job_requirements<System, Type, Args...>, System, Type,
				  Args...> {
	using Super = job_requirements<observer_job_requirements<System, Type, Args...>, System,
				       Type, Args...>;
	using Super::Super;
	friend struct Super;

    private:
	template <typename Func> void set_work(Type &task, Func func)
	{
		task.work = [func](entt::observer &o, ecs_registry &reg, Args &&... args) {
			func(o, reg, std::forward<Args>(args)...);
		};
	}
};

template <typename System, typename Type, typename...> struct view_job_requirements;

template <typename System, typename Type, typename... Args, typename... Included,
	  typename... Excluded>
struct view_job_requirements<System, Type, entt::type_list<Args...>, entt::type_list<Included...>,
			     entt::type_list<Excluded...>>
	: public job_requirements<
		  view_job_requirements<System, Type, entt::type_list<Args...>,
					entt::type_list<Included...>, entt::type_list<Excluded...>>,
		  System, Type, Args...> {
	using Super = job_requirements<
		view_job_requirements<System, Type, entt::type_list<Args...>,
				      entt::type_list<Included...>, entt::type_list<Excluded...>>,
		System, Type, Args...>;
	using Super::Super;
	friend struct Super;

    private:
	template <typename Func> void set_work(Type &task, Func func)
	{
		task.work = [func](ecs_registry &reg, Args &&... args) {
			func(reg.view<Included...>(entt::exclude<Excluded...>), reg,
			     std::forward<Args>(args)...);
		};
	}
};

template <typename... Type> struct job_exclude_t : public entt::type_list<Type...> {
};

template <typename System, typename... Args>
struct task_query_constructor<System, entt::type_list<Args...>> {
	task_query_constructor(System *sys, ecs_registry *registry)
		: system{ sys }, registry_{ registry }
	{
	}

	template <typename... Excluded> auto with_none() const noexcept
	{
		return task_query_constructor<System, entt::type_list<Args...>,
					      job_exclude_t<Excluded...>>{ system };
	};

	template <typename... Included> auto with_all() const noexcept
	{
		return task_query_constructor<System, entt::type_list<Args...>, job_exclude_t<>,
					      Included...>{ system };
	}

	template <typename Type> auto updated() const noexcept
	{
		return (task_reactive_constructor<System, entt::type_list<Args...>>{ system,
										     registry_ })
			.updated<Type>();
	}

	template <typename... Included, typename... Excluded>
	auto entered_group(job_exclude_t<Excluded...> = {}) const noexcept
	{
		return (task_reactive_constructor<System, entt::type_list<Args...>>{ system,
										     registry_ })
			.entered_group<Included...>(job_exclude_t<Excluded...>{});
	}

    private:
	System *system;
	ecs_registry *registry_;
};

template <typename System, typename... Args, typename... Excluded, typename... Included>
struct task_query_constructor<System, entt::type_list<Args...>, job_exclude_t<Excluded...>,
			      Included...> {
	task_query_constructor(System *sys) : system{ sys }
	{
	}

	template <typename... ExExcluded> auto with_none() const noexcept
	{
		return task_query_constructor<System, entt::type_list<Args...>,
					      job_exclude_t<Excluded..., ExExcluded...>,
					      Included...>{ system };
	};

	template <typename... ExIncluded> auto with_all() const noexcept
	{
		return task_query_constructor<System, entt::type_list<Args...>,
					      job_exclude_t<Excluded...>, Included...,
					      ExIncluded...>{ system };
	}

	using requirement_t =
		view_job_requirements<System, view_task<Args...>, entt::type_list<Args...>,
				      entt::type_list<Included...>, entt::type_list<Excluded...>>;

	template <typename... Reads> requirement_t add_read()
	{
		return requirement_t{ system, make_task() }.add_read<Reads...>();
	}

	template <typename... Writes> requirement_t add_write()
	{
		return requirement_t{ system, make_task() }.add_write<Writes...>();
	}

    private:
	TSharedPtr<view_task<Args...>> make_task()
	{
		return MakeShared<view_task<Args...>>();
	}

	System *system;
};

template <typename System, typename... Args>
struct task_reactive_constructor<System, entt::type_list<Args...>> {
	task_reactive_constructor(System *sys, ecs_registry *registry)
		: system{ sys }, registry_{ registry }
	{
	}

	template <typename Type>
	auto updated() const -> task_reactive_constructor<System, entt::type_list<Args...>,
							  decltype(entt::collector.update<Type>())>
	{
		using namespace entt;
		return task_reactive_constructor<System, entt::type_list<Args...>,
						 decltype(collector.update<Type>())>{ system,
										      registry_ };
	};

	template <typename... Included, typename... Excluded>
	auto entered_group(job_exclude_t<Excluded...> = {}) const -> task_reactive_constructor<
		System, entt::type_list<Args...>,
		decltype(entt::collector.group<Included...>(entt::exclude<Excluded...>))>
	{
		using namespace entt;
		return task_reactive_constructor<System, entt::type_list<Args...>,
						 decltype(collector.group<Included...>(
							 exclude<Excluded...>))>{ system,
										  registry_ };
	}

    private:
	System *system;
	ecs_registry *registry_;
};

template <typename System, typename... Args, typename... Reject, typename... Require,
	  typename... Rule, typename... Other>
struct task_reactive_constructor<
	System, entt::type_list<Args...>,
	entt::basic_collector<
		entt::matcher<entt::type_list<Reject...>, entt::type_list<Require...>, Rule...>,
		Other...>> {
	using collector_t = entt::basic_collector<
		entt::matcher<entt::type_list<Reject...>, entt::type_list<Require...>, Rule...>,
		Other...>;

	task_reactive_constructor(System *sys, ecs_registry *registry)
		: system{ sys }, registry_{ registry }
	{
	}

	template <typename Type>
	auto updated() const -> task_reactive_constructor<System, entt::type_list<Args...>,
							  decltype(collector_t::update<Type>())>
	{
		return task_reactive_constructor<System, entt::type_list<Args...>,
						 decltype(collector_.update<Type>())>{ system,
										       registry_ };
	};

	template <typename... Included, typename... Excluded>
	auto entered_group(job_exclude_t<Excluded...> = {}) const -> task_reactive_constructor<
		System, entt::type_list<Args...>,
		decltype(collector_t::group<Included...>(entt::exclude<Excluded...>))>
	{
		return task_reactive_constructor<System, entt::type_list<Args...>,
						 decltype(collector_.group<Included...>(
							 entt::exclude<Excluded...>))>{ system,
											registry_ };
	}

	template <typename... AllOf, typename... NoneOf>
	auto where(job_exclude_t<NoneOf...> = {}) const noexcept
	{
		return task_reactive_constructor<System, entt::type_list<Args...>,
						 decltype(collector_.where<AllOf...>(
							 entt::exclude<NoneOf...>))>{ system,
										      registry_ };
	}

	using requirement_t = observer_job_requirements<System, observer_task<Args...>, Args...>;

	template <typename... Reads> requirement_t add_read()
	{
		return requirement_t(system, make_task()).add_read<Reads...>();
	}

	template <typename... Writes> requirement_t add_write()
	{
		return requirement_t(system, make_task()).add_write<Writes...>();
	}

    private:
	auto make_task()
	{
		auto ptr = MakeShared<observer_task<Args...>>();
		observer_task<Args...> &task = *ptr;

		using wrapper_t = observer_wraper<System, entt::type_info<collector_t>::id()>;
		registry_->set<wrapper_t>(*registry_, collector_);

		task.observer_retrieval = [c = collector_](ecs_registry &reg, entt::observer *&ob) {
			wrapper_t &wrapper = reg.ctx<wrapper_t>();
			ob = &wrapper.observer;
		};

		return ptr;
	}

	collector_t collector_;
	System *system;
	ecs_registry *registry_;
};

} // namespace tc
