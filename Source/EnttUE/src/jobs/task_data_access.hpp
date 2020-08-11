#pragma once

#include "CoreMinimal.h"
#include "entt/core/type_info.hpp"

namespace tc
{
enum class access_t { none = 0, read = 1, write = 2, read_write = read | write };

static constexpr access_t operator&(access_t lhs, access_t rhs)
{
	using underlying_t = std::underlying_type_t<access_t>;
	return static_cast<access_t>(
		static_cast<underlying_t>(lhs)
		& static_cast<underlying_t>(rhs));
}

static constexpr access_t operator|(access_t lhs, access_t rhs)
{
	using underlying_t = std::underlying_type_t<access_t>;
	return static_cast<access_t>(static_cast<underlying_t>(lhs)
		| static_cast<underlying_t>(rhs));
}

namespace internal
{
template <access_t Access> struct data_access {
	static constexpr access_t access = Access;
};

template <typename...> struct data_read : public data_access<access_t::read> {
};

template <typename...> struct data_written : public data_access<access_t::write> {
};

} // namespace internal

template <typename... Data> static constexpr internal::data_read<Data...> data_read{};
template <typename... Data> static constexpr internal::data_written<Data...> data_written{};

struct task_data_access {
    public:
	
	task_data_access() = default;

	template <typename... DataRead, typename... DataWritten>
	task_data_access(internal::data_read<DataRead...> read,
			 internal::data_written<DataWritten...> written = data_written)
	{
		add<DataRead...>(internal::access_t::read);
		add<DataWritten...>(internal::access_t::write);
	}

	template <typename... DataWritten>
	task_data_access(internal::data_written<DataWritten...> written = data_written)
		: task_data_access(data_read, written)
	{
	}

	template <typename... Data> void add(access_t access)
	{
		if ((access & access_t::read) != access_t::none) {
			(reads.Add(entt::type_info<Data>::id()), ...);
		}

		if ((access & access_t::write) != access_t::none) {
			(writes.Add(entt::type_info<Data>::id()), ...);
		}
	}

	const TSet<uint32_t> &get_reads() const
	{
		return reads;
	}

	const TSet<uint32_t> &get_writes() const
	{
		return writes;
	}

    private:
	TSet<uint32_t> reads;
	TSet<uint32_t> writes;
};
} // namespace tc