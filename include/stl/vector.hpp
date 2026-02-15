#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.hpp"

#include <climits>
#include <cstddef>

namespace sjtu
{
/**
 * a data container like std::vector
 * store data in a successive memory and support random access.
 */
template<typename T>
class vector
{
	void* buffer_;
	T* data_;
	size_t size_;
	size_t capacity_;

	void expand() {
		capacity_ *= 2;
		void *new_buffer = operator new[](sizeof(T) * capacity_);
		T* new_data = static_cast<T*>(new_buffer);
		for (int i = 0; i < size_; i++) {
			new (&new_data[i]) T(data_[i]);
			data_[i].~T();
		}
		operator delete [](buffer_);
		buffer_ = new_buffer;
		data_ = new_data;
	}

public:
	/**
	 * TODO
	 * a type for actions of the elements of a vector, and you should write
	 *   a class named const_iterator with same interfaces.
	 */
	/**
	 * you can see RandomAccessIterator at CppReference for help.
	 */
	class const_iterator;
	class iterator
	{
	// The following code is written for the C++ type_traits library.
	// Type traits is a C++ feature for describing certain properties of a type.
	// For instance, for an iterator, iterator::value_type is the type that the
	// iterator points to.
	// STL algorithms and containers may use these type_traits (e.g. the following
	// typedef) to work properly. In particular, without the following code,
	// @code{std::sort(iter, iter1);} would not compile.
	// See these websites for more information:
	// https://en.cppreference.com/w/cpp/header/type_traits
	// About value_type: https://blog.csdn.net/u014299153/article/details/72419713
	// About iterator_category: https://en.cppreference.com/w/cpp/iterator
	friend class vector;
	friend class const_iterator;

	public:
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using reference = T&;
		using iterator_category = std::output_iterator_tag;

	private:
		/**
		 * TODO add data members
		 *   just add whatever you want.
		 */
		pointer ptr_;
		pointer begin_;
		pointer end_;

		iterator(const pointer& ptr, const pointer& begin, const pointer& end) : ptr_(ptr), begin_(begin), end_(end) {}

	public:
		/**
		 * return a new iterator which pointer n-next elements
		 * as well as operator-
		 */
		iterator operator+(const int &n) const
		{
			//TODO
			return iterator(ptr_ + n, begin_, end_);
		}
		iterator operator-(const int &n) const
		{
			//TODO
			return iterator(ptr_ - n, begin_, end_);
		}
		// return the distance between two iterators,
		// if these two iterators point to different vectors, throw invaild_iterator.
		int operator-(const iterator &rhs) const
		{
			//TODO
			if (begin_ != rhs.begin_ || end_ != rhs.end_) {
				throw invalid_iterator();
			}
			return rhs.ptr_ - ptr_;
		}
		iterator& operator+=(const int &n)
		{
			//TODO
			ptr_ += n;
			return *this;
		}
		iterator& operator-=(const int &n)
		{
			//TODO
			ptr_ -= n;
			return *this;
		}
		/**
		 * TODO iter++
		 */
		iterator operator++(int) {
			iterator temp(ptr_, begin_, end_);
			ptr_++;
			return temp;
		}
		/**
		 * TODO ++iter
		 */
		iterator& operator++() {
			ptr_++;
			return *this;
		}
		/**
		 * TODO iter--
		 */
		iterator operator--(int) {
			iterator temp(ptr_, begin_, end_);
			ptr_--;
			return temp;
		}
		/**
		 * TODO --iter
		 */
		iterator& operator--() {
			ptr_--;
			return *this;
		}
		/**
		 * TODO *it
		 */
		T& operator*() const{
			return *ptr_;
		}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory address).
		 */
		bool operator==(const iterator &rhs) const {
			return ptr_ == rhs.ptr_;
		}
		bool operator==(const const_iterator &rhs) const {
			return ptr_ == rhs.ptr_;
		}
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const {
			return ptr_ != rhs.ptr_;
		}
		bool operator!=(const const_iterator &rhs) const {
			return ptr_ != rhs.ptr_;
		}
	};
	/**
	 * TODO
	 * has same function as iterator, just for a const object.
	 */
	class const_iterator
	{
	friend class vector;
	friend class iterator;
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using reference = T&;
		using iterator_category = std::output_iterator_tag;

	private:
		/*TODO*/
		pointer ptr_;
		pointer begin_;
		pointer end_;

		const_iterator(const pointer& ptr, const pointer& begin, const pointer& end) : ptr_(ptr), begin_(begin), end_(end) {}

	public:
		/**
		 * return a new iterator which pointer n-next elements
		 * as well as operator-
		 */
		const_iterator operator+(const int &n) const
		{
			//TODO
			return const_iterator(ptr_ + n, begin_, end_);
		}
		const_iterator operator-(const int &n) const
		{
			//TODO
			return const_iterator(ptr_ - n, begin_, end_);
		}
		// return the distance between two iterators,
		// if these two iterators point to different vectors, throw invaild_iterator.
		int operator-(const const_iterator &rhs) const
		{
			//TODO
			if (begin_ != rhs.begin_ || end_ != rhs.end_) {
				throw invalid_iterator();
			}
			return rhs.ptr_ - ptr_;
		}
		const_iterator& operator+=(const int &n)
		{
			//TODO
			ptr_ += n;
			return *this;
		}
		const_iterator& operator-=(const int &n)
		{
			//TODO
			ptr_ -= n;
			return *this;
		}
		/**
		 * TODO iter++
		 */
		const_iterator operator++(int) {
			const_iterator temp(ptr_, begin_, end_);
			ptr_++;
			return temp;
		}
		/**
		 * TODO ++iter
		 */
		const_iterator& operator++() {
			ptr_++;
			return *this;
		}
		/**
		 * TODO iter--
		 */
		const_iterator operator--(int) {
			const_iterator temp(ptr_, begin_, end_);
			ptr_--;
			return temp;
		}
		/**
		 * TODO --iter
		 */
		const_iterator& operator--() {
			ptr_--;
			return *this;
		}
		/**
		 * TODO *it
		 */
		const T& operator*() const{
			return *ptr_;
		}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory address).
		 */
		bool operator==(const iterator &rhs) const {
			return ptr_ == rhs.ptr_;
		}
		bool operator==(const const_iterator &rhs) const {
			return ptr_ == rhs.ptr_;
		}
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const {
			return ptr_ != rhs.ptr_;
		}
		bool operator!=(const const_iterator &rhs) const {
			return ptr_ != rhs.ptr_;
		}
	};
	/**
	 * TODO Constructs
	 * At least two: default constructor, copy constructor
	 */
	vector() : size_(0), capacity_(10) {
		buffer_ = operator new[](sizeof(T) * capacity_);
		data_ = static_cast<T*>(buffer_);
	}
	vector(const vector &other) : size_(other.size_), capacity_(other.capacity_) {
		buffer_ = operator new[](sizeof(T) * capacity_);
		data_ = static_cast<T*>(buffer_);
		for (int i = 0; i < size_; i++) {
			new (&data_[i]) T(other.data_[i]);
		}
	}
	/**
	 * TODO Destructor
	 */
	~vector() {
		for (int i = 0; i < size_; i++) {
			data_[i].~T();
		}
		operator delete [](buffer_);
	}
	/**
	 * TODO Assignment operator
	 */
	vector &operator=(const vector &other) {
		if (this == &other) {
			return *this;
		}
		for (int i = 0; i < size_; i++) {
			data_[i].~T();
		}
		operator delete [](buffer_);
		size_ = other.size_;
		capacity_ = other.capacity_;
		buffer_ = operator new[](sizeof(T) * capacity_);
		data_ = static_cast<T*>(buffer_);
		for (int i = 0; i < size_; i++) {
			new (&data_[i]) T(other.data_[i]);
		}
		return *this;
	}
	/**
	 * assigns specified element with bounds checking
	 * throw index_out_of_bound if pos is not in [0, size)
	 */
	T & at(const size_t &pos) {
		if (pos < 0 || pos >= size_) {
			throw index_out_of_bound();
		}
		return data_[pos];
	}
	const T & at(const size_t &pos) const {
		if (pos < 0 || pos >= size_) {
			throw index_out_of_bound();
		}
		return data_[pos];
	}
	/**
	 * assigns specified element with bounds checking
	 * throw index_out_of_bound if pos is not in [0, size)
	 * !!! Pay attentions
	 *   In STL this operator does not check the boundary but I want you to do.
	 */
	T & operator[](const size_t &pos) {
		if (pos < 0 || pos >= size_) {
			throw index_out_of_bound();
		}
		return data_[pos];
	}
	const T & operator[](const size_t &pos) const {
		if (pos < 0 || pos >= size_) {
			throw index_out_of_bound();
		}
		return data_[pos];
	}
	/**
	 * access the first element.
	 * throw container_is_empty if size == 0
	 */
	const T & front() const {
		if (size_ == 0) {
			throw container_is_empty();
		}
		return data_[0];
	}
	/**
	 * access the last element.
	 * throw container_is_empty if size == 0
	 */
	const T & back() const {
		if (size_ == 0) {
			throw container_is_empty();
		}
		return data_[size_ - 1];
	}
	/**
	 * returns an iterator to the beginning.
	 */
	iterator begin() {
		return iterator(data_, data_, data_ + size_);
	}
	const_iterator begin() const {
		return const_iterator(data_, data_, data_ + size_);
	}
	const_iterator cbegin() const {
		return const_iterator(data_, data_, data_ + size_);
	}
	/**
	 * returns an iterator to the end.
	 */
	iterator end() {
		return iterator(data_ + size_, data_, data_ + size_);
	}
	const_iterator end() const {
		return const_iterator(data_ + size_, data_, data_ + size_);
	}
	const_iterator cend() const {
		return const_iterator(data_ + size_, data_, data_ + size_);
	}
	/**
	 * checks whether the container is empty
	 */
	bool empty() const {
		return size_ == 0;
	}
	/**
	 * returns the number of elements
	 */
	size_t size() const {
		return size_;
	}
	/**
	 * clears the contents
	 */
	void clear() {
		for (int i = 0; i < size_; i++) {
			data_[i].~T();
		}
		size_ = 0;
	}
	/**
	 * inserts value before pos
	 * returns an iterator pointing to the inserted value.
	 */
	iterator insert(iterator pos, const T &value) {
		size_t ind = pos.ptr_ - data_;
		if (size_ == capacity_) {
			expand();
		}
		if (size_ == 0) {
			new (data_) T(value);
			size_++;
			return iterator(data_, data_, data_ + size_);
		}
		new (&data_[size_]) T(data_[size_ - 1]);
		for (int i = size_ - 1; i > ind; i--) {
			data_[i] = data_[i - 1];
		}
		data_[ind] = value;
		size_++;
		return iterator(data_ + ind, data_, data_ + size_);
	}
	/**
	 * inserts value at index ind.
	 * after inserting, this->at(ind) == value
	 * returns an iterator pointing to the inserted value.
	 * throw index_out_of_bound if ind > size (in this situation ind can be size because after inserting the size will increase 1.)
	 */
	iterator insert(const size_t &ind, const T &value) {
		if (ind < 0 || ind > size_) {
			throw index_out_of_bound();
		}
		if (size_ == capacity_) {
			expand();
		}
		if (size_ == 0) {
			new (data_) T(value);
			size_++;
			return iterator(data_, data_, data_ + size_);
		}
		new (&data_[size_]) T(data_[size_ - 1]);
		for (int i = size_ - 1; i > ind; i--) {
			data_[i] = data_[i - 1];
		}
		data_[ind] = value;
		size_++;
		return iterator(data_ + ind, data_, data_ + size_);
	}
	/**
	 * removes the element at pos.
	 * return an iterator pointing to the following element.
	 * If the iterator pos refers the last element, the end() iterator is returned.
	 */
	iterator erase(iterator pos) {
		size_t ind = pos.ptr_ - data_;
		data_[ind].~T();
		for (int i = ind; i < size_ - 1; i++) {
			data_[i] = data_[i + 1];
		}
		size_--;
		return iterator(data_ + ind, data_, data_ + size_);
	}
	/**
	 * removes the element with index ind.
	 * return an iterator pointing to the following element.
	 * throw index_out_of_bound if ind >= size
	 */
	iterator erase(const size_t &ind) {
		if (ind >= size_) {
			throw index_out_of_bound();
		}
		data_[ind].~T();
		for (int i = ind; i < size_ - 1; i++) {
			data_[i] = data_[i + 1];
		}
		size_--;
		return iterator(data_ + ind, data_, data_ + size_);
	}
	/**
	 * adds an element to the end.
	 */
	void push_back(const T &value) {
		if (size_ == capacity_) {
			expand();
		}
		new (&data_[size_]) T(value);
		size_++;
	}
	/**
	 * remove the last element from the end.
	 * throw container_is_empty if size() == 0
	 */
	void pop_back() {
		if (size_ == 0) {
			throw container_is_empty();
		}
		data_[size_ - 1].~T();
		size_--;
	}
};


}

#endif
