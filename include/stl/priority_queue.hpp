#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

namespace sjtu {
/**
 * @brief a container like std::priority_queue which is a heap internal.
 * **Exception Safety**: The `Compare` operation might throw exceptions for certain data.
 * In such cases, any ongoing operation should be terminated, and the priority queue should be restored to its original state before the operation began.
 */
template<typename T, class Compare = std::less<T>>
class priority_queue {
	struct heap_node {
		T value;
		heap_node *left;
		heap_node *right;
		heap_node() : left(nullptr), right(nullptr) {}
		heap_node(const T& val) : value(val), left(nullptr), right(nullptr) {}
	};

	heap_node *root_;
	size_t size_;
	Compare comp_;

	void release(heap_node *pos) {
		if (pos->left) {
			release(pos->left);
		}
		if (pos->right) {
			release(pos->right);
		}
		delete pos;
	}

	void copy(heap_node *&dst, heap_node *src) {
		if (src->left) {
			dst->left = new heap_node(src->left->value);
			copy(dst->left, src->left);
		}
		if (src->right) {
			dst->right = new heap_node(src->right->value);
			copy(dst->right, src->right);
		}
	}

	heap_node* merge(heap_node *a, heap_node *b) {
		if (a == nullptr) {
			return b;
		}
		if (b == nullptr) {
			return a;
		}
		bool should_swap;
		try {
			should_swap = comp_(a->value, b->value);
		}
		catch(...) {
			throw;
		}
		if (should_swap) {
			heap_node *temp = a;
			a = b;
			b = temp;
		}
		heap_node *ar = a->right;
		heap_node *merged;
		try {
			merged = merge(ar, b);
		}
		catch(...) {
			a->right = ar;
			throw;
		}
		a->right = a->left;
		a->left = merged;
		return a;
	}

public:
	/**
	 * @brief default constructor
	 */
	priority_queue() : root_(nullptr), size_(0), comp_() {}

	/**
	 * @brief copy constructor
	 * @param other the priority_queue to be copied
	 */
	priority_queue(const priority_queue &other) : root_(nullptr), size_(other.size_), comp_() {
		root_ = new heap_node(other.root_->value);
		copy(root_, other.root_);
	}

	/**
	 * @brief deconstructor
	 */
	~priority_queue() {
		if (root_) {
			release(root_);
		}
	}

	/**
	 * @brief Assignment operator
	 * @param other the priority_queue to be assigned from
	 * @return a reference to this priority_queue after assignment
	 */
	priority_queue &operator=(const priority_queue &other) {
		if (this == &other) {
			return *this;
		}
		if (root_) {
			release(root_);
		}
		size_ = other.size_;
		root_ = new heap_node(other.root_->value);
		copy(root_, other.root_);
		return *this;
	}

	/**
	 * @brief get the top element of the priority queue.
	 * @return a reference of the top element.
	 * @throws container_is_empty if empty() returns true
	 */
	const T & top() const {
		if (root_ == nullptr) {
			throw container_is_empty();
		}
		else {
			return root_->value;
		}
	}

	/**
	 * @brief push new element to the priority queue.
	 * @param e the element to be pushed
	 */
	void push(const T &e) {
		heap_node *new_node = new heap_node(e);
		root_ = merge(root_, new_node);
		size_++;
	}

	/**
	 * @brief delete the top element from the priority queue.
	 * @throws container_is_empty if empty() returns true
	 */
	void pop() {
		if (root_ == nullptr) {
			throw container_is_empty();
		}
		heap_node *del = root_;
		root_ = merge(root_->left, root_->right);
		delete del;
		size_--;
	}

	/**
	 * @brief return the number of elements in the priority queue.
	 * @return the number of elements.
	 */
	size_t size() const {
		return size_;
	}

	/**
	 * @brief check if the container is empty.
	 * @return true if it is empty, false otherwise.
	 */
	bool empty() const {
		return size_ == 0;
	}

	/**
	 * @brief merge another priority_queue into this one.
	 * The other priority_queue will be cleared after merging.
	 * The complexity is at most O(logn).
	 * @param other the priority_queue to be merged.
	 */
	void merge(priority_queue &other) {
		root_ = merge(root_, other.root_);
		size_ += other.size_;
		other.size_ = 0;
		other.root_ = nullptr;
	}
};

}

#endif