#include "Position.h"

#include "Position.ostream.h"

#include <gtest/gtest.h>

TEST(Position, generic) {
    using P = text::Position;
    using L = text::Line;
    using C = text::Column;

    constexpr auto t = [] {
        auto p = P{};
        static_assert(P{} == P{L{1}, C{1}});
        p.nextColumn();
        p.nextLine();
        p.nextColumn();
        p.nextColumn();
        return p;
    }();
    static_assert(t == P{L{2}, C{3}});
}

TEST(Position, tabStops) {
    using P = text::Position;
    using L = text::Line;
    using C = text::Column;

    for (auto c = 1u; c <= 4u; ++c) {
        auto p = P{L{3}, C{c}};
        p.nextTabstop(C{4});
        ASSERT_EQ(p, (P{L{3}, C{5}}));
    }
    for (auto c = 5u; c <= 8u; ++c) {
        auto p = P{L{3}, C{c}};
        p.nextTabstop(C{4});
        ASSERT_EQ(p, (P{L{3}, C{9}}));
    }
}
