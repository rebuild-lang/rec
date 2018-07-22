#include "TypeList.h"

#include <gtest/gtest.h>

TEST(typelist, indexOf) {
    using TestList = meta::TypeList<int, float, double>;

    static_assert(TestList::indexOf<int>() == 0);
    static_assert(TestList::indexOf<double>() == 2);

    static_assert(TestList::indexOf<char>() == TestList::npos);
    static_assert(TestList{} == meta::TypeList<int, float, double>{});
}

TEST(typelist, indexOfMultpiple) {
    using TestList = meta::TypeList<int, float, int>;

    static_assert(TestList::indexOf<int>() == 0);
}

TEST(typelist, contains) {
    using TestList = meta::TypeList<int, float, double>;

    static_assert(TestList::containsPred(
        [](auto v) { return std::is_same_v<decltype(v), float> || std::is_same_v<decltype(v), char>; }));

    static_assert(TestList::containsPred([](auto v) { return sizeof(v) >= sizeof(double); }));
}

TEST(typelist, join) {
    using A = meta::TypeList<int, float>;
    using B = meta::TypeList<char, float>;
    static_assert(A{} != B{});

    constexpr auto c = A::join(B{});

    static_assert(c == meta::TypeList<int, float, char, float>{});
}

struct TooBig {
    template<class T>
    constexpr bool operator()(meta::Type<T>) const {
        return sizeof(T) >= sizeof(char[4]);
    }
};
TEST(typelist, filterpred) {
    using TestList = meta::TypeList<char[2], char[4], char[3], char[5]>;

    constexpr auto filtered = TestList::filterPred<TooBig>();

    static_assert(filtered == meta::TypeList<char[2], char[3]>{});
}
