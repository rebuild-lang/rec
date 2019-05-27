#pragma once
#include <cinttypes>

namespace text {

struct Line {
    using This = Line;

    uint32_t v{1}; // 1 is first line

    constexpr Line() = default;
    constexpr explicit Line(uint32_t x)
        : v(x) {}

    constexpr auto operator++() noexcept -> Line& {
        v++;
        return *this;
    }

    constexpr bool operator==(const This& o) const { return v == o.v; }
    constexpr bool operator!=(const This& o) const { return v != o.v; }

    constexpr bool operator>(const This& o) const { return v > o.v; }
    constexpr bool operator<(const This& o) const { return v < o.v; }
    constexpr bool operator>=(const This& o) const { return v >= o.v; }
    constexpr bool operator<=(const This& o) const { return v <= o.v; }
};

struct Column {
    using This = Column;

    uint32_t v{1}; // 1 is first column

    constexpr Column() = default;
    constexpr explicit Column(uint32_t x)
        : v(x) {}

    constexpr Column& operator++() noexcept {
        v++;
        return *this;
    }

    constexpr bool operator==(const This& o) const { return v == o.v; }
    constexpr bool operator!=(const This& o) const { return v != o.v; }

    constexpr bool operator<(const This& o) const { return v < o.v; }
    constexpr bool operator>(const This& o) const { return v > o.v; }
    constexpr bool operator>=(const This& o) const { return v >= o.v; }
    constexpr bool operator<=(const This& o) const { return v <= o.v; }
};

struct Position {
    using This = Position;

    Line line{};
    Column column{};

    constexpr void nextColumn() noexcept { ++column; }
    constexpr void nextTabstop(Column tabstop) noexcept { //
        column.v += tabstop.v - ((column.v - 1) % tabstop.v);
    }
    constexpr void nextLine() noexcept {
        ++line;
        column = {};
    }

    constexpr bool operator==(const This& o) const { return line == o.line && column == o.column; }
    constexpr bool operator!=(const This& o) const { return !(*this == o); }

    constexpr bool operator>(const This& o) const { return line > o.line || (line == o.line && column > o.column); }
    constexpr bool operator>=(const This& o) const { return line > o.line || (line == o.line && column >= o.column); }
};

} // namespace text
