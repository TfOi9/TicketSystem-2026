#ifndef HASH_TABLE_HPP
#define HASH_TABLE_HPP

#include <cstddef>
#include <type_traits>
#include <utility>

namespace sjtu {

namespace hash {

constexpr size_t initial_hash_value = 14695981039346656037ull;
constexpr size_t hash_mul = 1099511628211ull;

template<typename T>
struct MemoryHash {
    size_t operator()(const T& obj) const noexcept {
        const char *str = reinterpret_cast<const char *>(&obj);
        size_t len = sizeof(obj) / sizeof(char);
        size_t val = initial_hash_value;
        for (size_t i = 0; i < len; i++) {
            val ^= static_cast<size_t>(str[i]);
            val *= hash_mul;
        }
        return val;
    }
};

} // namespace hash

template<typename KeyType, typename ValueType = void, typename Hasher = hash::MemoryHash<KeyType>>
class HashTable {
private:
    struct EmptyValue {};
    using StoredValue = std::conditional_t<std::is_void_v<ValueType>, EmptyValue, ValueType>;

    struct Node {
        KeyType key_;
        StoredValue val_;
        Node *next_;

        Node(const KeyType& key, Node *next = nullptr) : key_(key), val_(), next_(next) {}

        Node(const KeyType& key, const StoredValue& val, Node *next = nullptr) : key_(key), val_(val), next_(next) {}
    };

    Node **bucket_;
    size_t bucket_count_;
    size_t size_;
    double max_load_;
    Hasher hasher_;

    void release_link(Node *head) {
        Node *pos = head;
        while (pos) {
            Node *del = pos;
            pos = pos->next_;
            delete del;
        }
    }

    size_t hash(const KeyType& key) const {
        return hasher_(key) % bucket_count_;
    }

    void ensure_capacity() {
        if (need_rehash()) {
            rehash(bucket_count_ == 0 ? 1 : bucket_count_ * 2);
        }
    }

public:
    using node_type = Node;

    HashTable(size_t bucket_count = 16, double max_load = 1.0) : bucket_count_(bucket_count ? bucket_count : 1), size_(0), max_load_(max_load) {
        bucket_ = new Node*[bucket_count_]();
    }

    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;
    HashTable(HashTable&&) = delete;
    HashTable& operator=(HashTable&&) = delete;

    ~HashTable() {
        for (size_t i = 0; i < bucket_count_; i++) {
            release_link(bucket_[i]);
        }
        delete []bucket_;
    }

    Node* find(const KeyType& key) const {
        size_t index = hash(key);
        Node *pos = bucket_[index];
        while (pos) {
            if (pos->key_ == key) {
                return pos;
            }
            pos = pos->next_;
        }
        return nullptr;
    }

    Node* insert(const KeyType& key) {
        size_t index = hash(key);
        Node *pos = bucket_[index];
        Node *prev = nullptr;
        while (pos) {
            if (pos->key_ == key) {
                return pos;
            }
            prev = pos;
            pos = pos->next_;
        }

        Node *new_node = new Node(key);
        if (prev) {
            prev->next_ = new_node;
        }
        else {
            bucket_[index] = new_node;
        }
        size_++;
        ensure_capacity();
        return new_node;
    }

    template<typename V = ValueType, typename = std::enable_if_t<!std::is_void_v<V>>>
    Node* insert(const KeyType& key, const V& val) {
        size_t index = hash(key);
        Node *pos = bucket_[index];
        Node *prev = nullptr;
        while (pos) {
            if (pos->key_ == key) {
                return pos;
            }
            prev = pos;
            pos = pos->next_;
        }

        Node *new_node = new Node(key, val);
        if (prev) {
            prev->next_ = new_node;
        }
        else {
            bucket_[index] = new_node;
        }
        size_++;
        ensure_capacity();
        return new_node;
    }

    Node* erase(const KeyType& key) {
        size_t index = hash(key);
        Node *pos = bucket_[index];
        Node *prev = nullptr;
        while (pos) {
            if (pos->key_ == key) {
                if (prev == nullptr) {
                    bucket_[index] = pos->next_;
                }
                else {
                    prev->next_ = pos->next_;
                }
                Node *del = pos;
                pos = pos->next_;
                delete del;
                size_--;
                return pos;
            }
            else {
                prev = pos;
                pos = pos->next_;
            }
        }
        return nullptr;
    }

    template<typename V = ValueType, typename = std::enable_if_t<!std::is_void_v<V>>>
    Node* erase(const KeyType& key, const V& val) {
        size_t index = hash(key);
        Node *pos = bucket_[index];
        Node *prev = nullptr;
        while (pos) {
            if (pos->key_ == key && pos->val_ == val) {
                if (prev == nullptr) {
                    bucket_[index] = pos->next_;
                }
                else {
                    prev->next_ = pos->next_;
                }
                Node *del = pos;
                pos = pos->next_;
                delete del;
                size_--;
                return pos;
            }
            else {
                prev = pos;
                pos = pos->next_;
            }
        }
        return nullptr;
    }

    void rehash(size_t new_bucket_count) {
        if (new_bucket_count == 0) {
            return;
        }
        Node **new_bucket = new Node*[new_bucket_count]();
        for (size_t i = 0; i < bucket_count_; i++) {
            Node *pos = bucket_[i];
            while (pos) {
                Node *next_node = pos->next_;
                size_t new_index = hasher_(pos->key_) % new_bucket_count;
                pos->next_ = new_bucket[new_index];
                new_bucket[new_index] = pos;
                pos = next_node;
            }
            bucket_[i] = nullptr;
        }
        delete []bucket_;
        bucket_ = new_bucket;
        bucket_count_ = new_bucket_count;
    }

    double load_factor() const {
        return bucket_count_ == 0 ? 0.0 : static_cast<double>(size_) / static_cast<double>(bucket_count_);
    }

    bool need_rehash() const {
        return load_factor() > max_load_;
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    void clear() {
        for (size_t i = 0; i < bucket_count_; i++) {
            release_link(bucket_[i]);
            bucket_[i] = nullptr;
        }
        size_ = 0;
    }

    void set_max_load(double max_load) {
        max_load_ = max_load;
        ensure_capacity();
    }

    // iteration helpers used by unordered_map/set wrappers
    Node* first_node() const {
        for (size_t i = 0; i < bucket_count_; i++) {
            if (bucket_[i]) {
                return bucket_[i];
            }
        }
        return nullptr;
    }

    Node* next_node(const Node* current) const {
        if (current == nullptr) {
            return nullptr;
        }
        if (current->next_) {
            return current->next_;
        }
        size_t index = hash(current->key_);
        for (size_t i = index + 1; i < bucket_count_; i++) {
            if (bucket_[i]) {
                return bucket_[i];
            }
        }
        return nullptr;
    }

};

} // namespace sjtu

#endif // HASH_TABLE_HPP