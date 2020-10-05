#pragma once

#include "CoreMinimal.h"
#include "core/fwd.hpp"
#include "fwd.hpp"
#include "entt/entity/observer.hpp"
#include "executable_task.hpp"
#include "job_requirements_builder.hpp"

namespace tc
{
template <typename Type, entt::id_type Id> struct observer_wraper {
	template <typename Collector>
	observer_wraper(ecs_registry &reg, Collector collector) : observer{ reg, collector }
	{
	}
	entt::observer observer;
};

template <typename... Args> struct reactive_task : public executable_task<ecs_registry &, Args...> {
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

template <typename System, typename Type, typename... Args>
struct observer_job_requirements
	: public job_requirements_builder<observer_job_requirements<System, Type, Args...>, System,
					  Type, Args...> {
	using Super = job_requirements_builder<observer_job_requirements<System, Type, Args...>,
					       System, Type, Args...>;
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

namespace internal
{
	template <typename...> struct entity_reactive_query_builder;

template <typename System, typename... Args>
struct entity_reactive_query_builder<System, entt::type_list<Args...>> {
	entity_reactive_query_builder(System *sys, ecs_registry *registry)
		: system{ sys }, registry_{ registry }
	{
	}

	template <typename Type>
	auto updated() const -> entity_reactive_query_builder<System, entt::type_list<Args...>,
							  decltype(entt::collector.update<Type>())>
	{
		using namespace entt;
		return entity_reactive_query_builder<System, entt::type_list<Args...>,
						 decltype(collector.update<Type>())>{ system,
										      registry_ };
	};

	template <typename... Included, typename... Excluded>
	auto entered_group(job_exclude_t<Excluded...> = {}) const -> entity_reactive_query_builder<
		System, entt::type_list<Args...>,
		decltype(entt::collector.group<Included...>(entt::exclude<Excluded...>))>
	{
		using namespace entt;
		return entity_reactive_query_builder<System, entt::type_list<Args...>,
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
struct entity_reactive_query_builder<
	System, entt::type_list<Args...>,
	entt::basic_collector<
		entt::matcher<entt::type_list<Reject...>, entt::type_list<Require...>, Rule...>,
		Other...>> {
	using collector_t = entt::basic_collector<
		entt::matcher<entt::type_list<Reject...>, entt::type_list<Require...>, Rule...>,
		Other...>;

	entity_reactive_query_builder(System *sys, ecs_registry *registry)
		: system{ sys }, registry_{ registry }
	{
	}

	template <typename Type>
	auto updated() const -> entity_reactive_query_builder<System, entt::type_list<Args...>,
							  decltype(collector_t::update<Type>())>
	{
		return entity_reactive_query_builder<System, entt::type_list<Args...>,
						 decltype(collector_.update<Type>())>{ system,
										       registry_ };
	};

	template <typename... Included, typename... Excluded>
	auto entered_group(job_exclude_t<Excluded...> = {}) const -> entity_reactive_query_builder<
		System, entt::type_list<Args...>,
		decltype(collector_t::group<Included...>(entt::exclude<Excluded...>))>
	{
		return entity_reactive_query_builder<System, entt::type_list<Args...>,
						 decltype(collector_.group<Included...>(
							 entt::exclude<Excluded...>))>{ system,
											registry_ };
	}

	template <typename... AllOf, typename... NoneOf>
	auto where(job_exclude_t<NoneOf...> = {}) const noexcept
	{
		return entity_reactive_query_builder<System, entt::type_list<Args...>,
						 decltype(collector_.where<AllOf...>(
							 entt::exclude<NoneOf...>))>{ system,
										      registry_ };
	}

	using requirement_t = observer_job_requirements<System, reactive_task<Args...>, Args...>;

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
		auto ptr = MakeShared<reactive_task<Args...>>();
		reactive_task<Args...> &task = *ptr;

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
} // namespace internal

} // namespace tc