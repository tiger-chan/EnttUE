#pragma once

namespace tc
{
    template <typename... task_args_t>
    class task_builder;

    template <typename... task_args_t>
    class task_registar
    {
        using task_builder_t = task_builder<task_args_t...>;

    public:
        virtual void add_graph(TSharedPtr<task_graph<task_args_t...>> &&graph) = 0;
        virtual task_builder_t build_task_graph(FName name, uint32 priority) = 0;
    };

} // namespace tc
