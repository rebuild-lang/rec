#include "View.ostream.h"

#include <gtest/gtest.h>

using strings::Counter;
using strings::View;

TEST(view, basic) {
    constexpr auto v = View{"foobar"};

    static_assert(v.byteCount() == strings::Counter{6});
    static_assert(v.firstBytes(Counter{6}) == v);
    static_assert(v.skipBytes(Counter{3}).byteCount() == strings::Counter{3});
    static_assert(v.skipBytes(Counter{3}).isContentEqual(View{"bar"}));

    auto vr = View{"foobar"};
    ASSERT_EQ(vr.byteCount().v, 6);
    ASSERT_EQ(vr.firstBytes<6>(), vr);

    auto s = strings::String{"foobar"};
    ASSERT_TRUE(vr.isContentEqual(s));

    static_assert(sizeof(strings::OptionalView) == sizeof(strings::View));
}

TEST(view, compare_view) {
    auto cv = strings::CompareView{"foobar"};

    auto s = strings::String{"foobar"};
    ASSERT_EQ(cv, s);

    // EXPECT_EQ(cv, (strings::String{"fowl"})); // trigger assert failure output
}
