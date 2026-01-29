#ifndef TYPE_HELPER_HPP
#define TYPE_HELPER_HPP

#include <type_traits>
#include <utility>

namespace sjtu {
#define __GENERATE_OPERATOR_CHECK(name, op) \
    template<typename T, typename = void> \
    struct has_operator_##name##_impl : std::false_type {}; \
    template<typename T> \
    struct has_operator_##name##_impl<T, std::void_t<decltype(std::declval<T>() op std::declval<T>())>> : std::true_type {}; \
    template<typename T> \
    inline constexpr bool has_operator_##name##_v = has_operator_##name##_impl<T>::value;

#define GENERATE_OPERATOR_CHECKS \
    __GENERATE_OPERATOR_CHECK(equal, ==) \
    __GENERATE_OPERATOR_CHECK(not_equal, !=) \
    __GENERATE_OPERATOR_CHECK(less, <) \
    __GENERATE_OPERATOR_CHECK(greater, >) \
    __GENERATE_OPERATOR_CHECK(less_equal, <=) \
    __GENERATE_OPERATOR_CHECK(greater_equal, >=)

GENERATE_OPERATOR_CHECKS

#undef GENERATE_OPERATOR_CHECKS

template<typename Type>
struct is_pod : std::bool_constant<
    std::is_standard_layout_v<Type> &&
    std::is_trivially_copyable_v<Type> &&
    std::is_trivially_destructible_v<Type> &&
    std::is_trivially_copy_constructible_v<Type> &&
    std::is_trivially_move_constructible_v<Type> &&
    std::is_trivially_copy_assignable_v<Type> &&
    std::is_trivially_move_assignable_v<Type>
> {};

template<typename T>
inline constexpr bool is_pod_v = is_pod<T>::value;

} // namespace sjtu

#endif // TYPE_HELPER_HPP