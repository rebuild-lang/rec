#pragma once

#include "parser/block/blockToken.h"

namespace parser::expression {

using BlockToken = block::Token;
using BlockLine = block::TokenLine;

struct BlockLineView {
    using This = BlockLineView;

    BlockLineView(const BlockLine *line)
        : line(line) {}

    BlockLineView() = default;
    BlockLineView(const This &) = default;
    BlockLineView(This &&) = default;
    This &operator=(const This &) = default;
    This &operator=(This &&) = default;

    operator bool() const { return line && index < line->size(); }
    bool hasNext() const { return index + 1 < line->size(); }

    auto current() const -> decltype(auto) { return (*line)[index]; }
    auto next() const -> decltype(auto) { return (*line)[index + 1]; }

    bool operator==(const This &o) const { return line == o.line && index == o.index; }
    bool operator!=(const This &o) const { return !(*this == o); }

    auto operator++() & -> This & {
        index++;
        return *this;
    }

private:
    const BlockLine *line{};
    size_t index{};
};

} // namespace parser::expression
