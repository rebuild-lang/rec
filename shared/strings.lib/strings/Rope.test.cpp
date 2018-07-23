#include "Rope.ostream.h"

#include <gtest/gtest.h>

TEST(rope, basic) {
    auto r = strings::Rope{};

    ASSERT_TRUE(r.isEmpty());
    ASSERT_EQ(r.byteCount().v, 0);
    ASSERT_EQ(static_cast<strings::String>(r), strings::String{});
    ASSERT_EQ(r, strings::View{""});

    r += strings::CodePoint{'f'};
    r += strings::View{"oo"};
    r += strings::String{"bar"};

    ASSERT_FALSE(r.isEmpty());
    ASSERT_EQ(r.byteCount().v, 6);
    ASSERT_EQ(strings::to_string(r), strings::String{"foobar"});
    ASSERT_EQ(r, strings::View{"foobar"});

    // EXPECT_EQ(r, strings::View{"fowl"}); // trigger failing assert output
}
