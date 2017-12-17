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
    uint32_t v{1}; // 1 is first line

    constexpr line_t() = default;
    constexpr explicit line_t(uint32_t x)
        : v(x) {}

    constexpr line_t &operator++() noexcept {
        v++;
        return *this;
    }

    constexpr bool operator==(const line_t &o) const { return v == o.v; }
    constexpr bool operator!=(const line_t &o) const { return v != o.v; }
};

struct column_t {
    uint32_t v{1}; // 1 is first column

    constexpr column_t() = default;
    constexpr explicit column_t(uint32_t x)
        : v(x) {}

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

    bool operator==(const text_range &o) const {
        return file == o.file && text.content_equals(o.text) && begin_position == o.begin_position &&
               end_position == o.end_position;
    }
    bool operator!=(const text_range &o) const { return !(*this == o); }
};

inline auto operator<<(std::ostream &out, const text_range &r) -> std::ostream & {
    out << r.begin_position << r.text << r.end_position << " in " << (r.file ? r.file->filename : string_t("<null>"));
    return out;
}

} // namespace scanner
