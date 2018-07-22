#include "View.ostream.h"

#include <gtest/gtest.h>

TEST(view, basic) {
    constexpr auto v = strings::View{"foobar"};

    static_assert(v.byteCount() == strings::Counter{6});
    static_assert(v.front<6>() == v);

    auto vr = strings::View{"foobar"};
    ASSERT_EQ(vr.byteCount().v, 6);
    ASSERT_EQ(vr.front<6>(), vr);

    auto s = strings::String{"foobar"};
    ASSERT_TRUE(vr.isContentEqual(s));

    auto cp = vr.pullCodePoint();
    ASSERT_EQ(cp, strings::CodePoint{'f'});
    ASSERT_EQ(vr.byteCount().v, 5);

    static_assert(sizeof(strings::OptionalView) == sizeof(strings::View));
}

TEST(view, compare_view) {
    auto cv = strings::CompareView{"foobar"};

    auto s = strings::String{"foobar"};
    ASSERT_EQ(cv, s);

    // EXPECT_EQ(cv, (strings::String{"fowl"})); // trigger assert failure output
}

TEST(view, pull_bom) {
    auto cv = strings::View{
        "\xEF\xBB\xBF"
        "foo"};

    auto pv = cv;
    ASSERT_TRUE(pv.pullBom());
    ASSERT_EQ(pv.byteCount().v, 3);
    ASSERT_EQ(pv, (strings::View{cv.begin() + 3, cv.end()}));
}
