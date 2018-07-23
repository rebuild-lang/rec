#include "Counter.ostream.h"

#include <gtest/gtest.h>

TEST(counter, basic) {
    constexpr auto c = strings::Counter{};

    static_assert(c.v == 0);
    static_assert(c == strings::Counter{0});

    constexpr auto c2 = c + strings::Counter{23};
    static_assert(c2.v == 23);
    static_assert(c2 == strings::Counter{23});
}

TEST(counter, runtime) {
    auto c = strings::Counter{};

    // EXPECT_EQ(c, strings::Counter{23}); // Trigger failing assert output
}
