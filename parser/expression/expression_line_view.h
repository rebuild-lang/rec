#pragma once

#include "parser/block/block_token.h"

namespace parser::expression {

using block_token = block::token;
using block_line = block::line;

struct line_view_t {
    using this_t = line_view_t;

    line_view_t(const block_line *line)
        : line(line) {}

    line_view_t(const this_t &) = default;
    line_view_t(this_t &&) = default;
    this_t &operator=(const this_t &) = default;
    this_t &operator=(this_t &&) = default;

    operator bool() const { return index < line->size(); }
    bool has_next() const { return index + 1 < line->size(); }

    auto current() const -> decltype(auto) { return (*line)[index]; }
    auto next() const -> decltype(auto) { return (*line)[index + 1]; }

    bool operator==(const this_t &o) const { return line == o.line && index == o.index; }
    bool operator!=(const this_t &o) const { return !(*this == o); }

    auto operator++() & -> this_t & {
        index++;
        return *this;
    }

private:
    const block_line *line;
    size_t index{};
};

} // namespace parser::expression
