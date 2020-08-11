#include "directed_graph.hpp"
#include <tuple>
#include <algorithm>
#include <unordered_map>

namespace tc
{
directed_graph::directed_graph(uint32_t node_count) : count_{ node_count }
{
	const int32_t matrix_count = node_count * node_count;
	adjacencies_.AddDefaulted(matrix_count);
	id_to_index_.Reserve(node_count);
	index_to_id_.Reserve(node_count);
}

void directed_graph::add(entt::id_type id, const TArray<entt::id_type> &dependencies)
{
	index_to_id_.FindOrAdd(id_to_index_.Num(), id);
	auto new_id = id_to_index_.FindOrAdd(id, id_to_index_.Num());

	for (const auto dep_id : dependencies) {
		index_to_id_.FindOrAdd(id_to_index_.Num(), dep_id);
		auto out_id = id_to_index_.FindOrAdd(dep_id, id_to_index_.Num());
		adjacencies_[get_index(new_id, out_id)] = 1;
	}
}

std::string directed_graph::print_dependency_count() const
{
	using pair_t = TPair<entt::id_type, uint32_t>;
	TArray<pair_t> counts;
	for (auto kv : id_to_index_) {
		auto &pair = counts.Emplace_GetRef(kv.Key, 0);
		for (auto x = 0u; x < count_; ++x) {
			pair.Value += adjacencies_[get_index(x, kv.Value)];
		}
	}

	counts.Sort([](const pair_t &lhs, const pair_t &rhs) { return lhs.Value < rhs.Value; });

	std::stringstream stream{};
	for (auto &kv : counts) {
		stream << std::setw(15) << kv.Key << ": " << std::setw(5) << kv.Value << std::endl;
	}

	return stream.str();
}

std::string directed_graph::to_string() const
{
	std::stringstream stream{};

	for (auto y = 0u; y < count_; ++y) {
		stream << std::setw(15) << index_to_id_[y];
		for (auto x = 0u; x < count_; ++x) {
			stream << std::setw(3) << std::to_string(adjacencies_[get_index(x, y)])
			       << "|";
		}
		stream << std::endl << std::setw(15) << " ";
		stream << std::setw(count_ * 4) << std::setfill('-') << "-" << std::endl
		       << std::setfill(' ');
	}

	return stream.str();
}

std::vector<sortable_graph::data_t> sortable_graph::sorted_nodes() const
{
	if (!sorted_.empty()) {
		return sorted_;
	}

	auto sort_func = [](const data_t &lhs, const data_t &rhs) {
		return lhs.dependencies.size() > rhs.dependencies.size();
	};

	std::unordered_map<entt::id_type, std::unordered_set<entt::id_type>> dependencies;
	std::vector<data_t> counts;
	for (auto kv : id_to_index_) {
		auto &data = counts.emplace_back(data_t{ kv.Key });
		auto dep = dependencies.emplace(kv.Key, std::unordered_set<entt::id_type>{}).first;
		for (auto x = 0u; x < count_; ++x) {
			bool adjacent = adjacencies_[get_index(x, kv.Value)] != 0;
			if (!adjacent) {
				continue;
			}
			auto adj_id = index_to_id_[x];
			data.dependencies.emplace(index_to_id_[x]);
			dep->second.emplace(adj_id);
		}
	}

	std::sort(std::begin(counts), std::end(counts), sort_func);

	sorted_.clear();
	sorted_.resize(count_);
	size_t index = sorted_.size();
	while (!counts.empty()) {
		auto data = counts.back();
		counts.pop_back();
		if (data.dependencies.size() == 0) {
			sorted_[--index] = data_t{ data.id, dependencies[data.id] };

			for (auto &other_data : counts) {
				other_data.dependencies.erase(data.id);
			}
		} else {
			auto next_without_dependents = std::find_if(
				std::begin(counts), std::end(counts),
				[](const data_t &d) { return d.dependencies.size() == 0; });
			if (next_without_dependents == std::end(counts)) {
				sorted_.clear();
				break;
			}
			counts.emplace_back(std::move(data));
			std::sort(std::begin(counts), std::end(counts), sort_func);
		}
	}

	return sorted_;
}

} // namespace tc
