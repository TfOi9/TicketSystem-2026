#ifndef COMPARATOR_HPP
#define COMPARATOR_HPP

#include <type_traits>
#include <cstddef>
    
#include "../config.hpp"

namespace sjtu {
#define MEMORYHASH_TYPE MemoryHash<FixedType>
#define MEMORYHASH_TEMPLATE_ARGS template<typename FixedType>

#define COMPARATOR_TYPE Comparator<FixedType>
#define COMPARATOR_TEMPLATE_ARGS template<typename FixedType>

MEMORYHASH_TEMPLATE_ARGS
class MemoryHash {
private:
    FixedType data_;
    hash_t hash1_;
    hash_t hash2_;
    constexpr static size_t sizeofT = sizeof(FixedType);
public:
    MemoryHash(const FixedType& data);

    hash_t hash1() const;

    hash_t hash2() const;

};

MEMORYHASH_TEMPLATE_ARGS
MEMORYHASH_TYPE::MemoryHash(const FixedType& data) : data_(data) {
    hash1_ = 0;
    hash2_ = 0;
    const unsigned char *bytes = reinterpret_cast<const unsigned char *>(&data_);
    for (size_t i = 0; i < sizeofT; i++) {
        hash1_ = (hash1_ * HASH_BASE1 + hash_t(bytes[i])) % HASH_MOD1;
        hash2_ = (hash2_ * HASH_BASE2 + hash_t(bytes[i])) % HASH_MOD2;
    }
}

MEMORYHASH_TEMPLATE_ARGS
hash_t MEMORYHASH_TYPE::hash1() const {
    return hash1_;
}

MEMORYHASH_TEMPLATE_ARGS
hash_t MEMORYHASH_TYPE::hash2() const {
    return hash2_;
}

COMPARATOR_TEMPLATE_ARGS
class Comparator {
public:
    int operator()(const FixedType& a, const FixedType& b) const;
};

COMPARATOR_TEMPLATE_ARGS
int COMPARATOR_TYPE::operator()(const FixedType &a, const FixedType &b) const {
    MEMORYHASH_TYPE hash_a(a), hash_b(b);
    if (hash_a.hash1() == hash_b.hash1()) {
        if (hash_a.hash2() == hash_b.hash2()) {
            return 0;
        }
        return (hash_a.hash2() > hash_b.hash2()) ? 1 : -1;
    }
    return (hash_a.hash1() > hash_b.hash1()) ? 1 : -1;
}

} // namespace sjtu

#endif // COMPARATOR_HPP