#pragma once

#include "strings/utf8_string.h"
#include "strings/utf8_view.h"

#include <ostream>

namespace scanner {

using string_t = strings::utf8_string;
using view_t = strings::utf8_view;

struct file_t {
    string_t filename;
    string_t content;
};

struct line_t {
    uint32_t v;

    constexpr line_t()
        : v(1) {} // start with line 1
    constexpr explicit line_t(uint32_t x)
        : v(x) {}

    constexpr line_t(const line_t &) = default;
    constexpr line_t &operator=(const line_t &) = default;
    constexpr line_t(line_t &&) = default;
    constexpr line_t &operator=(line_t &&) = default;

    constexpr line_t &operator++() noexcept {
        v++;
        return *this;
    }

    constexpr bool operator==(const line_t &o) const { return v == o.v; }
    constexpr bool operator!=(const line_t &o) const { return v != o.v; }
};

struct column_t {
    uint32_t v;

    constexpr column_t()
        : v(1) {} // start with column 1
    constexpr explicit column_t(uint32_t x)
        : v(x) {}

    constexpr column_t(const column_t &) = default;
    constexpr column_t &operator=(const column_t &) = default;
    constexpr column_t(column_t &&) = default;
    constexpr column_t &operator=(column_t &&) = default;

    constexpr column_t &operator++() noexcept {
        v++;
        return *this;
    }

    constexpr bool operator==(const column_t &o) const { return v == o.v; }
    constexpr bool operator!=(const column_t &o) const { return v != o.v; }

    constexpr bool operator<(const column_t &o) const { return v < o.v; }
    constexpr bool operator>(const column_t &o) const { return v > o.v; }
    constexpr bool operator>=(const column_t &o) const { return v >= o.v; }
    constexpr bool operator<=(const column_t &o) const { return v <= o.v; }
};

struct position_t {
    line_t line;
    column_t column;

    constexpr void next_column() noexcept { ++column; }
    constexpr void next_tabstop(column_t tabstop) noexcept { column.v += tabstop.v - (column.v % tabstop.v); }
    constexpr void next_line() noexcept {
        ++line;
        column = {};
    }

    constexpr bool operator==(const position_t &o) const { return line == o.line && column == o.column; }
    constexpr bool operator!=(const position_t &o) const { return !(*this == o); }
};

inline auto operator<<(std::ostream &out, const position_t &p) -> std::ostream & {
    return out << '[' << p.line.v << ';' << p.column.v << ']';
}

struct text_range {
    const file_t *file = nullptr;
    view_t text;
    position_t begin_position;
    position_t end_position;
};

inline auto operator<<(std::ostream &out, const text_range &r) -> std::ostream & {
    out << r.begin_position << r.text << r.end_position << " in " << (r.file ? r.file->filename : string_t("<null>"));
    return out;
}

} // namespace scanner
