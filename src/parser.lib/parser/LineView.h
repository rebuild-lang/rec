#pragma once
#include "nesting/Token.h"

namespace parser {

using BlockToken = nesting::Token;
using nesting::BlockLine;

struct BlockLineView {
    using This = BlockLineView;

    BlockLineView() = default;
    explicit BlockLineView(const BlockLine* line)
        : line(line) {}

    explicit operator bool() const { return line && index < line->tokens.size(); }
    bool hasNext() const { return index + 1 < line->tokens.size(); }

    auto current() const -> decltype(auto) { return line->tokens[index]; }
    auto next() const -> decltype(auto) { return line->tokens[index + 1]; }

    bool operator==(const This& o) const { return line == o.line && index == o.index; }
    bool operator!=(const This& o) const { return !(*this == o); }

    auto operator++() & -> This& {
        index++;
        return *this;
    }

private:
    const BlockLine* line{};
    size_t index{};
};

} // namespace parser
