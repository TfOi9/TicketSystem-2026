#ifndef UNORDERED_SET_HPP
#define UNORDERED_SET_HPP

#include <cstddef>

#include "hash_table.hpp"

namespace sjtu {

template<typename KeyType, typename Hasher = hash::MemoryHash<KeyType>>
class unordered_set {
private:
    using Table = HashTable<KeyType, void, Hasher>;
    Table table_;

    struct key_ref {
        const KeyType *key;
    };

public:
    using size_type = std::size_t;

    class const_iterator;
    class iterator {
        friend class const_iterator;
    private:
        Table *table_ = nullptr;
        typename Table::node_type *node_ = nullptr;
        key_ref ref_ {nullptr};

        void refresh() {
            ref_.key = node_ ? &node_->key_ : nullptr;
        }
    public:
        iterator() = default;
        iterator(Table *table, typename Table::node_type *node) : table_(table), node_(node) { refresh(); }

        key_ref& operator*() { return ref_; }
        const key_ref& operator*() const { return ref_; }
        key_ref* operator->() { return &ref_; }
        const key_ref* operator->() const { return &ref_; }

        iterator& operator++() {
            node_ = table_ ? table_->next_node(node_) : nullptr;
            refresh();
            return *this;
        }

        bool operator==(const iterator &rhs) const { return node_ == rhs.node_; }
        bool operator!=(const iterator &rhs) const { return !(*this == rhs); }
    };

    class const_iterator {
    private:
        const Table *table_ = nullptr;
        const typename Table::node_type *node_ = nullptr;
        key_ref ref_ {nullptr};

        void refresh() {
            ref_.key = node_ ? &node_->key_ : nullptr;
        }
    public:
        const_iterator() = default;
        const_iterator(const Table *table, const typename Table::node_type *node) : table_(table), node_(node) { refresh(); }
        const_iterator(const iterator &it) : table_(it.table_), node_(it.node_) { refresh(); }

        const key_ref& operator*() const { return ref_; }
        const key_ref* operator->() const { return &ref_; }

        const_iterator& operator++() {
            node_ = table_ ? table_->next_node(node_) : nullptr;
            refresh();
            return *this;
        }

        bool operator==(const const_iterator &rhs) const { return node_ == rhs.node_; }
        bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }
    };

    unordered_set(size_type bucket_count = 16) : table_(bucket_count) {}

    iterator begin() { return iterator(&table_, table_.first_node()); }
    iterator end() { return iterator(&table_, nullptr); }
    const_iterator begin() const { return const_iterator(&table_, table_.first_node()); }
    const_iterator end() const { return const_iterator(&table_, nullptr); }

    size_type size() const { return table_.size(); }
    bool empty() const { return table_.empty(); }

    iterator find(const KeyType &key) { return iterator(&table_, table_.find(key)); }
    const_iterator find(const KeyType &key) const { return const_iterator(&table_, table_.find(key)); }

    struct insert_result {
        iterator it;
        bool inserted;
    };

    insert_result insert(const KeyType &key) {
        auto node = table_.find(key);
        if (!node) {
            node = table_.insert(key);
            return {iterator(&table_, node), true};
        }
        return {iterator(&table_, node), false};
    }

    size_type erase(const KeyType &key) {
        return table_.erase(key) ? 1 : 0;
    }

    void clear() { table_.clear(); }
};

} // namespace sjtu

#endif // UNORDERED_SET_HPP
