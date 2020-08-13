#pragma once

#include "EnttUE.h"

namespace tc
{
struct job_handle;

class job_base;

class job_processor;

ENTTUE_API TUniquePtr<job_processor> make_processor(bool run_parallel = false);

struct task_data_access;

template <typename... Args> struct task;

template <typename System> class system_base;

struct directed_graph;

struct sortable_graph;

template <typename...> struct task_query_constructor;

template <typename...> struct task_reactive_constructor;

} // namespace tc

#include "entt/core/type_info.hpp"

template <typename Type> struct entt::type_index<Type> {
};
