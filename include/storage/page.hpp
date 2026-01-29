#ifndef PAGE_HPP
#define PAGE_HPP

#include "../config.hpp"
#include "../utils/comparator.hpp"
#include "../utils/type_helper.hpp"

namespace sjtu {
#define KEYPAIR_TYPE KeyPair<KeyType, ValueType>
#define KEYPAIR_TEMPLATE_ARGS template<typename KeyType, typename ValueType>

#define PAGE_TYPE Page<KeyType, ValueType>
#define PAGE_TEMPLATE_ARGS template<typename KeyType, typename ValueType>

KEYPAIR_TEMPLATE_ARGS
struct KeyPair {
    KeyType key_;
    ValueType val_;

    KeyPair() = default;

    KeyPair(const KeyPair&) = default;

    KeyPair(const KeyType& key, const ValueType& val) : key_(key), val_(val) {}

    ~KeyPair() = default;

    KeyPair& operator=(const KeyPair& oth) = default;
};

KEYPAIR_TEMPLATE_ARGS
bool operator==(const KEYPAIR_TYPE& a, const KEYPAIR_TYPE& b) {
    if constexpr (has_operator_equal_v<KeyType> && has_operator_equal_v<ValueType>) {
        return a.key_ == b.key_ && a.val_ == b.val_;
    }
    else {
        Comparator<KeyType> key_comp;
        Comparator<ValueType> val_comp;
        return key_comp(a.key_, b.key_) == 0 && val_comp(a.val_, b.val_) == 0;
    }
}

KEYPAIR_TEMPLATE_ARGS
bool operator!=(const KEYPAIR_TYPE& a, const KEYPAIR_TYPE& b) {
    return !(a == b);
}

KEYPAIR_TEMPLATE_ARGS
bool operator>(const KEYPAIR_TYPE& a, const KEYPAIR_TYPE& b) {
    if constexpr (has_operator_greater_v<KeyType> && has_operator_greater_v<ValueType>) {
        if (a.key_ == b.key_) {
            return a.val_ > b.val_;
        }
        return a.key_ > b.key_;
    } else {
        Comparator<KeyType> key_comp;
        Comparator<ValueType> val_comp;
        int k = key_comp(a.key_, b.key_);
        if (k == 0) return val_comp(a.val_, b.val_) > 0;
        return k > 0;
    }
}

KEYPAIR_TEMPLATE_ARGS
bool operator<(const KEYPAIR_TYPE& a, const KEYPAIR_TYPE& b) {
    if constexpr (has_operator_less_v<KeyType> && has_operator_less_v<ValueType>) {
        if (a.key_ == b.key_) {
            return a.val_ < b.val_;
        }
        return a.key_ < b.key_;
    } else {
        Comparator<KeyType> key_comp;
        Comparator<ValueType> val_comp;
        int k = key_comp(a.key_, b.key_);
        if (k == 0) return val_comp(a.val_, b.val_) < 0;
        return k < 0;
    }
}

KEYPAIR_TEMPLATE_ARGS
bool operator>=(const KEYPAIR_TYPE& a, const KEYPAIR_TYPE& b) {
    return !(a < b);
}

KEYPAIR_TEMPLATE_ARGS
bool operator<=(const KEYPAIR_TYPE& a, const KEYPAIR_TYPE& b) {
    return !(a > b);
}

enum class PageType {
    Invalid = 0,
    Leaf,
    Internal
};

PAGE_TEMPLATE_ARGS
struct Page {
    KEYPAIR_TYPE data_[PAGE_SLOT_COUNT + 2];
    diskpos_t ch_[PAGE_SLOT_COUNT + 2];
    PageType type_ = PageType::Invalid;
    diskpos_t fa_ = -1;
    diskpos_t left_ = -1;
    diskpos_t right_ = -1;
    size_t size_ = 0;

    Page() = default;

    Page(const Page&) = default;

    ~Page() = default;

    int lower_bound(const KEYPAIR_TYPE& kp) const;

    int lower_bound(const KeyType& key) const;

    KEYPAIR_TYPE front() const;

    KEYPAIR_TYPE back() const;

};

PAGE_TEMPLATE_ARGS
int PAGE_TYPE::lower_bound(const KEYPAIR_TYPE& kp) const {
    int l = 0, r = size_ - 1, mid = -1, ans = r;
    while (l <= r) {
        mid = (l + r) / 2;
        if (data_[mid] < kp) {
            l = mid + 1;
        }
        else {
            ans = mid;
            r = mid - 1;
        }
    }
    return ans;
}

PAGE_TEMPLATE_ARGS
int PAGE_TYPE::lower_bound(const KeyType& key) const {
    int l = 0, r = size_ - 1, mid = -1, ans = r;
    while (l <= r) {
        mid = (l + r) / 2;
        if (data_[mid].key_ < key) {
            l = mid + 1;
        }
        else {
            ans = mid;
            r = mid - 1;
        }
    }
    return ans;
}

PAGE_TEMPLATE_ARGS
KEYPAIR_TYPE PAGE_TYPE::front() const {
    if (!size_) {
        return KEYPAIR_TYPE();
    }
    else {
        return data_[0];
    }
}

PAGE_TEMPLATE_ARGS
KEYPAIR_TYPE PAGE_TYPE::back() const {
    if (!size_) {
        return KEYPAIR_TYPE();
    }
    else {
        return data_[size_ - 1];
    }
}

} // namespace sjtu

#endif // PAGE_HPP