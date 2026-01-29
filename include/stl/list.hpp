#ifndef LIST_HPP
#define LIST_HPP

#include <cstddef>
#include <utility> // only for std::move() and std::swap()

namespace sjtu {
template<typename T>
class list {
private:
    struct node {
        T data_;
        node *prev_;
        node *next_;

        node(const T& data = T()) : data_(data), prev_(nullptr), next_(nullptr) {}
        node(T&& data) : data_(std::move(data)), prev_(nullptr), next_(nullptr) {}
    };

    node *head_;
    node *tail_;
    size_t size_;

    void init_empty() {
        head_ = new node();
        tail_ = new node();
        head_->next_ = tail_;
        tail_->prev_ = head_;
        size_ = 0;
    }

    void link_between(node *pos, node *n) {
        n->prev_ = pos->prev_;
        n->next_ = pos;
        pos->prev_->next_ = n;
        pos->prev_ = n;
        ++size_;
    }

    node* unlink(node *n) {
        node *next = n->next_;
        n->prev_->next_ = n->next_;
        n->next_->prev_ = n->prev_;
        delete n;
        --size_;
        return next;
    }

    void copy_from(const list& oth) {
        for (const auto& val : oth) {
            push_back(val);
        }
    }

public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;

    class iterator {
    private:
        node *cur_;

    public:
        explicit iterator(node *cur = nullptr) : cur_(cur) {}

        T& operator*() const {
            return cur_->data_;
        }

        T* operator->() const {
            return &(cur_->data_);
        }

        iterator& operator++() {
            cur_ = cur_->next_;
            return *this;
        }

        iterator operator++(int) {
            iterator temp = *this;
            cur_ = cur_->next_;
            return temp;
        }

        iterator& operator--() {
            cur_ = cur_->prev_;
            return *this;
        }

        iterator operator--(int) {
            iterator temp = *this;
            cur_ = cur_->prev_;
            return temp;
        }

        bool operator==(const iterator& oth) const {
            return cur_ == oth.cur_;
        }

        bool operator!=(const iterator& oth) const {
            return cur_ != oth.cur_;
        }

        friend class list<T>;
    
    };

    class const_iterator {
    private:
        const node *cur_;

    public:
        explicit const_iterator(const node *cur = nullptr) : cur_(cur) {}

        const_iterator(const iterator& it) : cur_(it.cur_) {}

        const T& operator*() const {
            return cur_->data_;
        }

        const T* operator->() const {
            return &(cur_->data_);
        }

        const_iterator& operator++() {
            cur_ = cur_->next_;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator temp = *this;
            cur_ = cur_->next_;
            return temp;
        }

        const_iterator& operator--() {
            cur_ = cur_->prev_;
            return *this;
        }

        const_iterator operator--(int) {
            const_iterator temp = *this;
            cur_ = cur_->prev_;
            return temp;
        }

        bool operator==(const const_iterator& oth) const {
            return cur_ == oth.cur_;
        }

        bool operator!=(const const_iterator& oth) const {
            return cur_ != oth.cur_;
        }

        friend class list<T>;
    
    };

    class reverse_iterator {
    private:
        node *cur_;

    public:
        explicit reverse_iterator(node *cur = nullptr) : cur_(cur) {}

        iterator base() const {
            return iterator(cur_);
        }

        T& operator*() const {
            return cur_->prev_->data_;
        }

        T* operator->() const {
            return &(cur_->prev_->data_);
        }

        reverse_iterator& operator++() {
            cur_ = cur_->prev_;
            return *this;
        }

        reverse_iterator operator++(int) {
            reverse_iterator temp = *this;
            cur_ = cur_->prev_;
            return temp;
        }

        reverse_iterator& operator--() {
            cur_ = cur_->next_;
            return *this;
        }

        reverse_iterator operator--(int) {
            reverse_iterator temp = *this;
            cur_ = cur_->next_;
            return temp;
        }

        bool operator==(const reverse_iterator& oth) const {
            return cur_ == oth.cur_;
        }

        bool operator!=(const reverse_iterator& oth) const {
            return cur_ != oth.cur_;
        }

        friend class list<T>;
    };

    class const_reverse_iterator {
    private:
        const node *cur_;

    public:
        explicit const_reverse_iterator(const node *cur = nullptr) : cur_(cur) {}

        const_iterator base() const {
            return const_iterator(const_cast<node*>(cur_));
        }

        const T& operator*() const {
            return cur_->prev_->data_;
        }

        const T* operator->() const {
            return &(cur_->prev_->data_);
        }

        const_reverse_iterator& operator++() {
            cur_ = cur_->prev_;
            return *this;
        }

        const_reverse_iterator operator++(int) {
            const_reverse_iterator temp = *this;
            cur_ = cur_->prev_;
            return temp;
        }

        const_reverse_iterator& operator--() {
            cur_ = cur_->next_;
            return *this;
        }

        const_reverse_iterator operator--(int) {
            const_reverse_iterator temp = *this;
            cur_ = cur_->next_;
            return temp;
        }

        bool operator==(const const_reverse_iterator& oth) const {
            return cur_ == oth.cur_;
        }

        bool operator!=(const const_reverse_iterator& oth) const {
            return cur_ != oth.cur_;
        }

        friend class list<T>;
    };

    list() {
        init_empty();
    }

    explicit list(size_t count, const T& value = T()) {
        init_empty();
        while (count--) {
            push_back(value);
        }
    }

    template<typename InputIt>
    list(InputIt first, InputIt last) {
        init_empty();
        for (; first != last; ++first) {
            push_back(*first);
        }
    }

    list(const list& oth) {
        init_empty();
        copy_from(oth);
    }

    list(list&& oth) noexcept {
        head_ = oth.head_;
        tail_ = oth.tail_;
        size_ = oth.size_;
        oth.head_ = oth.tail_ = nullptr;
        oth.size_ = 0;
    }

    list& operator=(const list& oth) {
        if (this == &oth) return *this;
        clear();
        copy_from(oth);
        return *this;
    }

    list& operator=(list&& oth) noexcept {
        if (this == &oth) return *this;
        clear();
        delete head_;
        delete tail_;
        head_ = oth.head_;
        tail_ = oth.tail_;
        size_ = oth.size_;
        oth.head_ = oth.tail_ = nullptr;
        oth.size_ = 0;
        return *this;
    }

    ~list() {
        clear();
        delete head_;
        delete tail_;
    }

    iterator begin() {
        return iterator(head_->next_);
    }

    const_iterator begin() const {
        return const_iterator(head_->next_);
    }

    iterator end() {
        return iterator(tail_);
    }

    const_iterator end() const {
        return const_iterator(tail_);
    }

    const_iterator cbegin() const {
        return begin();
    }

    const_iterator cend() const {
        return end();
    }

    reverse_iterator rbegin() {
        return reverse_iterator(tail_);
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(tail_);
    }

    reverse_iterator rend() {
        return reverse_iterator(head_);
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(head_);
    }

    const_reverse_iterator crbegin() const {
        return rbegin();
    }

    const_reverse_iterator crend() const {
        return rend();
    }

    bool empty() const {
        return size_ == 0;
    }

    size_t size() const {
        return size_;
    }

    void assign(size_t count, const T& value) {
        clear();
        while (count--) push_back(value);
    }

    template<typename InputIt>
    void assign(InputIt first, InputIt last) {
        clear();
        for (; first != last; ++first) push_back(*first);
    }

    T& front() {
        return head_->next_->data_;
    }

    const T& front() const {
        return head_->next_->data_;
    }

    T& back() {
        return tail_->prev_->data_;
    }

    const T& back() const {
        return tail_->prev_->data_;
    }

    void clear() {
        if (!head_) return;
        node *cur = head_->next_;
        while (cur != tail_) {
            cur = unlink(cur);
        }
    }

    template<typename... Args>
    iterator emplace(iterator pos, Args&&... args) {
        node *n = new node(T(std::forward<Args>(args)...));
        link_between(pos.cur_, n);
        return iterator(n);
    }

    iterator insert(iterator pos, const T& value) {
        node *n = new node(value);
        link_between(pos.cur_, n);
        return iterator(n);
    }

    iterator insert(iterator pos, T&& value) {
        node *n = new node(std::move(value));
        link_between(pos.cur_, n);
        return iterator(n);
    }

    iterator insert(iterator pos, size_t count, const T& value) {
        iterator first_insert = pos;
        bool inserted = false;
        for (; count; --count) {
            iterator cur = insert(pos, value);
            if (!inserted) {
                first_insert = cur;
                inserted = true;
            }
            ++pos;
        }
        return inserted ? first_insert : pos;
    }

    template<typename InputIt>
    iterator insert(iterator pos, InputIt first, InputIt last) {
        iterator first_insert = pos;
        bool inserted = false;
        for (; first != last; ++first) {
            iterator cur = insert(pos, *first);
            if (!inserted) {
                first_insert = cur;
                inserted = true;
            }
            ++pos;
        }
        return inserted ? first_insert : pos;
    }

    iterator erase(iterator pos) {
        return iterator(unlink(pos.cur_));
    }

    iterator erase(iterator first, iterator last) {
        while (first != last) {
            first = erase(first);
        }
        return last;
    }

    void push_front(const T& value) {
        insert(begin(), value);
    }

    void push_front(T&& value) {
        insert(begin(), std::move(value));
    }

    template<typename... Args>
    void emplace_front(Args&&... args) {
        emplace(begin(), std::forward<Args>(args)...);
    }

    void push_back(const T& value) {
        insert(end(), value);
    }

    void push_back(T&& value) {
        insert(end(), std::move(value));
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        emplace(end(), std::forward<Args>(args)...);
    }

    void pop_front() {
        erase(begin());
    }

    void pop_back() {
        iterator it = end();
        --it;
        erase(it);
    }

    void resize(size_t count, const T& value = T()) {
        while (size_ > count) pop_back();
        while (size_ < count) push_back(value);
    }

    void remove(const T& value) {
        for (auto it = begin(); it != end(); ) {
            if (*it == value) it = erase(it); else ++it;
        }
    }

    void swap(list& oth) {
        std::swap(head_, oth.head_);
        std::swap(tail_, oth.tail_);
        std::swap(size_, oth.size_);
    }
};

template<typename T>
typename list<T>::iterator next(typename list<T>::iterator it, typename list<T>::difference_type n = 1) {
    while (n > 0) {
        ++it;
        --n;
    }
    while (n < 0) {
        --it;
        ++n;
    }
    return it;
}

template<typename T>
typename list<T>::const_iterator next(typename list<T>::const_iterator it, typename list<T>::difference_type n = 1) {
    while (n > 0) {
        ++it;
        --n;
    }
    while (n < 0) {
        --it;
        ++n;
    }
    return it;
}

template<typename T>
typename list<T>::reverse_iterator next(typename list<T>::reverse_iterator it, typename list<T>::difference_type n = 1) {
    while (n > 0) {
        ++it;
        --n;
    }
    while (n < 0) {
        --it;
        ++n;
    }
    return it;
}

template<typename T>
typename list<T>::const_reverse_iterator next(typename list<T>::const_reverse_iterator it, typename list<T>::difference_type n = 1) {
    while (n > 0) {
        ++it;
        --n;
    }
    while (n < 0) {
        --it;
        ++n;
    }
    return it;
}

} // namespace sjtu

#endif // LIST_HPP