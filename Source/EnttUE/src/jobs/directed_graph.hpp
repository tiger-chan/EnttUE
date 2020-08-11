#pragma once

#include "EnttUE.h"
#include "CoreMinimal.h"

#include <string>
#include <iomanip>
#include <sstream>
#include "entt/core/fwd.hpp"
#include "fwd.hpp"
#include <vector>
#include <unordered_set>

namespace tc
{
struct ENTTUE_API directed_graph {
	using node_t = uint8_t;

	directed_graph() = default;
	directed_graph(uint32_t node_count);

	void add(entt::id_type id, const TArray<entt::id_type> &dependencies);

	std::string print_dependency_count() const;

	std::string to_string() const;

    protected:
	inline node_t get_index(int32_t x, int32_t y) const
	{
		return y * count_ + x;
	}

	TArray<node_t> adjacencies_;
	TMap<entt::id_type, int32_t> id_to_index_;
	TMap<int32_t, entt::id_type> index_to_id_;
	uint32_t count_ = 0;
};

struct ENTTUE_API sortable_graph : public directed_graph {
	struct data_t {
		entt::id_type id;
		std::unordered_set<entt::id_type> dependencies;
	};
	using directed_graph::directed_graph;

	std::vector<data_t> sorted_nodes() const;

	template <typename Iter> void sort(Iter begin, Iter end)
	{
		auto sort_func = [](const data_t &lhs, const data_t &rhs) {
			return lhs.dependencies.size() > rhs.dependencies.size();
		};
		std::sort(begin, end, sort_func);
	}

    private:
	mutable std::vector<data_t> sorted_;
};

} // namespace tc
