#include <cassert>
#include <string>
#include "../include/utils/type_helper.hpp"

struct WithEq {
    int v;
    bool operator==(const WithEq& other) const { return v == other.v; }
};
struct WithOrd {
    int v;
    bool operator<(const WithOrd& other) const { return v < other.v; }
};
struct WithNeq {
    int v;
    bool operator!=(const WithNeq& other) const { return v != other.v; }
};
struct WithGt {
    int v;
    friend bool operator>(const WithGt& a, const WithGt& b) { return a.v > b.v; }
};
struct WithLe {
    int v;
    bool operator<=(const WithLe& other) const { return v <= other.v; }
};
struct WithEqInt {
    int v;
    int operator==(const WithEqInt& other) const { return v - other.v; }
};
struct WithEqNonConst {
    int v;
    bool operator==(WithEqNonConst& other) { return v == other.v; }
};
struct NoOps {
    int v;
};
struct NonPod {
    virtual ~NonPod() = default;
    int v;
};
struct DiskPod {
    int a;
    double b;
};
struct WithString {
    int id;
    std::string name; // 含有非平凡成员，不能直接落盘
};
struct WithArray {
    int arr[3];
};
struct WithPtr {
    int len;
    int* p;
};
struct Empty {};
struct NonTrivialDtor {
    ~NonTrivialDtor() {}
};
struct NoCopy {
    NoCopy() = default;
    NoCopy(const NoCopy&) = delete;
    int v;
};

int main() {
    using namespace sjtu;

    static_assert(has_operator_equal_v<WithEq>);
    static_assert(!has_operator_equal_v<NoOps>);
    static_assert(has_operator_less_v<WithOrd>);
    static_assert(!has_operator_greater_v<WithOrd>);
    static_assert(has_operator_not_equal_v<WithNeq>);
    static_assert(!has_operator_equal_v<WithNeq>);
    static_assert(has_operator_greater_v<WithGt>);
    static_assert(has_operator_less_equal_v<WithLe>);
    static_assert(has_operator_equal_v<WithEqInt>);
    static_assert(has_operator_equal_v<const WithEq>);
    static_assert(!has_operator_equal_v<const WithEqNonConst>);

    WithEq a{1}, b{1};
    assert(a == b);
    WithOrd c{1}, d{2};
    assert(c < d);
    WithNeq e{1}, f{2};
    assert(e != f);
    WithGt g{3}, h{1};
    assert(g > h);
    WithLe i{2}, j{2};
    assert(i <= j);

    return 0;
}