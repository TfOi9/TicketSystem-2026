#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cstddef>
#include <cstdint>

namespace sjtu {

typedef int64_t diskpos_t;

constexpr size_t PAGE_SLOT_COUNT = 200;
static_assert(PAGE_SLOT_COUNT % 2 == 0, "Slot count must be even!");

constexpr size_t CACHE_CAPACITY = 500;

typedef int64_t hash_t;

constexpr hash_t HASH_MOD1 = 998244353;
constexpr hash_t HASH_MOD2 = 1000000007;
constexpr hash_t HASH_BASE1 = 10007;
constexpr hash_t HASH_BASE2 = 9973;

} // namespace sjtu

#endif // CONFIG_HPP