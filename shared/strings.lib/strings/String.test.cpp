#include "String.ostream.h"

#include <array>
#include <gtest/gtest.h>

TEST(string, basic) {
    auto s = strings::String{"foo"};

    ASSERT_EQ(s.byteCount(), strings::Counter{3});
    ASSERT_FALSE(s.isEmpty());

    auto s2 = s;
    ASSERT_EQ(s, s2);

    auto s3 = strings::String{s.begin(), s.end() - 1};
    ASSERT_EQ(s3, (strings::String{'f', 'o'}));

    // trigger failing assert output
    // EXPECT_EQ(s2, s3);
    // EXPECT_EQ((strings::OptionalString{}), (strings::OptionalString{s3}));

    ASSERT_LT(s3, s);

    using Arr = std::array<strings::String::Char, 4>;
    auto d = Arr{0, 0, 0, 0};
    auto i = 0;
    for (auto c : s) d[i++] = c;
    ASSERT_EQ(d, (Arr{'f', 'o', 'o', 0}));

    static_assert(sizeof(strings::OptionalString) == sizeof(strings::String));
    ASSERT_EQ(strings::OptionalString{}, strings::String{});
}
