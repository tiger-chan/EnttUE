#pragma once

#include "CoreMinimal.h"
#include <iterator>

namespace tc
{
template <typename Data> class linked_list {
    public:
	class node {
		friend class linked_list;

	    public:
		node() = default;

		node(const Data &d) : data{ d }
		{
		}

		node(Data &&d) : data{ std::move(d) }
		{
		}

		Data data;
		node *next = nullptr;
		node *prev = nullptr;
	};

	class linked_list_iterator final {
		friend class linked_list;

	    public:
		using difference_type = size_t;
		using value_type = Data;
		using pointer = value_type *;
		using reference = value_type &;
		using iterator_category = std::bidirectional_iterator_tag;

		linked_list_iterator() = default;

		linked_list_iterator &operator++() noexcept
		{
			return ++index_, node_ = node_->next, *this;
		}

		linked_list_iterator operator++(int) noexcept
		{
			auto orig = *this;
			return operator++(), orig;
		}

		linked_list_iterator &operator--() noexcept
		{
			return --index_, node_ = node_->prev, *this;
		}

		linked_list_iterator operator--(int) noexcept
		{
			auto orig = *this;
			return operator--(), orig;
		}

		bool operator==(const linked_list_iterator &other) const noexcept
		{
			return other.index_ == index_;
		}

		bool operator!=(const linked_list_iterator &other) const noexcept
		{
			return !(*this == other);
		}

		bool operator<(const linked_list_iterator &other) const noexcept
		{
			return index_ > other.index_;
		}

		bool operator>(const linked_list_iterator &other) const noexcept
		{
			return index_ < other.index_;
		}

		bool operator<=(const linked_list_iterator &other) const noexcept
		{
			return !(*this > other);
		}

		bool operator>=(const linked_list_iterator &other) const noexcept
		{
			return !(*this < other);
		}

		pointer operator->() const
		{
			return &*operator*();
		}

		reference operator*() const
		{
			return node_->data;
		}

	    private:
		linked_list_iterator(node *node, size_t index) : node_{ node }, index_{ index }
		{
		}

		node *node_ = nullptr;
		size_t index_ = 0;
	};

	using iterator = linked_list_iterator;

	linked_list() = default;
	~linked_list()
	{
		clear_internal();
	}

	linked_list(const linked_list &) = delete;
	linked_list &operator=(const linked_list &) = delete;
	linked_list(linked_list &&other)
	{
		move_internal(other);
	}

	linked_list &operator=(linked_list &&other)
	{
		move_internal(other);

		return *this;
	}

	void emplace_back(const Data &data)
	{
		emplace_back_internal(new node(data));
	}

	void emplace_back(Data &&data)
	{
		emplace_back_internal(new node(std::move(data)));
	}

	inline void clear()
	{
		clear_internal();

		front_ = nullptr;
		back_ = nullptr;
		count_ = 0;
	}

	inline node *front()
	{
		return front_;
	}

	inline const node *const front() const
	{
		return front_;
	}

	inline size_t size() const
	{
		return count_;
	}

	inline bool empty() const
	{
		return count_ == 0;
	}

	iterator begin()
	{
		return iterator{ front_, 0 };
	}

	iterator end()
	{
		return iterator{ nullptr, count_ };
	}

    private:
	inline void clear_internal()
	{
		node *n = front_;
		while (n) {
			node *tmp = n;
			n = tmp->next;
			delete tmp;
		}
	}

	inline void move_internal(linked_list &other)
	{
		back_ = other.back_;
		front_ = other.front_;
		count_ = count_;

		other.back_ = nullptr;
		other.front_ = nullptr;
		other.count_ = 0;
	}

	inline void emplace_back_internal(node *new_node)
	{
		if (front_ == nullptr) {
			back_ = new_node;
			front_ = new_node;
		} else {
			new_node->prev = back_;
			back_->next = new_node;
			back_ = new_node;
		}
		++count_;
	}

	node *front_ = nullptr;
	node *back_ = nullptr;
	size_t count_ = 0;
};
} // namespace tc