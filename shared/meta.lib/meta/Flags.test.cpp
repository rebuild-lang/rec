#include "Flags.ostream.h"

#include <gtest/gtest.h>
#include <sstream>

enum class TestEnum { None, One = 1 << 0, Two = 1 << 1 };
using TestFlags = meta::Flags<TestEnum>;
META_FLAGS_OP(TestFlags)

auto operator<<(std::ostream& out, TestEnum e) -> std::ostream& {
    switch (e) {
    case TestEnum::One: return out << "One";
    case TestEnum::Two: return out << "Two";
    default: return out << "<unknown>";
    }
}

TEST(flags, constexpr) {
    constexpr auto v = TestFlags{};

    static_assert(false == v[TestEnum::One]);

    static_assert(v == TestFlags{});

    static_assert(false == v.all(TestEnum::One, TestEnum::Two));

    static_assert(v.none());
    static_assert(v.none(TestEnum::One, TestEnum::Two));

    constexpr auto v2 = v.set(TestEnum::One);
    static_assert(v2[TestEnum::One]);
    static_assert(v != v2);

    constexpr auto v3 = v | TestEnum::One;
    static_assert(v3.any(TestEnum::One, TestEnum::Two));
    static_assert(v3 == v2);

    static_assert(false == v3.none());
    static_assert(v3.none(TestEnum::Two));
}

TEST(flags, runtime) {
    auto v = TestFlags{};

    ASSERT_FALSE(v[TestEnum::One]);

    ASSERT_EQ(v, TestFlags{});
    ASSERT_FALSE(v.all(TestEnum::One, TestEnum::Two));

    v |= TestEnum::One | TestEnum::Two;
    ASSERT_EQ(v, (TestFlags{TestEnum::One, TestEnum::Two}));
    ASSERT_TRUE(v.all(TestEnum::One, TestEnum::Two));

    v &= TestEnum::One;
    ASSERT_EQ(v, TestEnum::One);

    auto called = bool{};
    v.each_set([&](auto v) {
        called = true;
        ASSERT_EQ(v, TestEnum::One);
    });
    ASSERT_TRUE(called);

    // ASSERT_EQ(v, (TestFlags{TestEnum::One, TestEnum::Two})); // Trigger Assert Failure Output
}

TEST(flags, ostream_empty) {
    auto ss = std::stringstream{};
    ss << TestFlags{};
    ASSERT_EQ(ss.str(), "<None>");
}

TEST(flags, ostream_one) {
    auto ss = std::stringstream{};
    ss << TestFlags{TestEnum::One};
    ASSERT_EQ(ss.str(), "One");
}

TEST(flags, ostream_two) {
    auto ss = std::stringstream{};
    ss << TestFlags{TestEnum::One, TestEnum::Two};
    ASSERT_EQ(ss.str(), "One | Two");
}
