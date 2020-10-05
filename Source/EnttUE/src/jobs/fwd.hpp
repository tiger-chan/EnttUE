#pragma once

#include "EnttUE.h"
#include "entt/core/type_traits.hpp"

namespace tc
{
struct job_handle;

class job_base;

class job_processor;

ENTTUE_API TUniquePtr<job_processor> make_processor(bool run_parallel = false);

struct task_data_access;

template <typename... Args> struct executable_task;

template <typename System> class system_base;

struct directed_graph;

struct sortable_graph;

template <typename...> struct entity_query_builder;

namespace internal
{
template <typename... Type> struct job_exclude_t : public entt::type_list<Type...> {
};
} // namespace internal

} // namespace tc

#include "entt/core/type_info.hpp"

template <typename Type> struct entt::type_index<Type> {
};
