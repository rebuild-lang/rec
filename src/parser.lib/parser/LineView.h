#pragma once
#include "nesting/Token.h"

namespace parser {

using BlockToken = nesting::Token;
using nesting::BlockLine;

struct BlockLineView {
    using This = BlockLineView;

    BlockLineView() = default;
    explicit BlockLineView(const BlockLine* line)
        : m_line(line) {}

    explicit operator bool() const { return m_line && m_index < m_line->tokens.size(); }
    bool hasNext() const { return m_index + 1 < m_line->tokens.size(); }

    auto current() const -> decltype(auto) { return m_line->tokens[m_index]; }
    auto next() const -> decltype(auto) { return m_line->tokens[m_index + 1]; }

    bool operator==(const This& o) const noexcept = default;
    bool operator<(const This& o) const { return m_line == o.m_line && m_index < o.m_index; }

    auto operator++() & -> This& {
        m_index++;
        return *this;
    }

    auto line() const -> const BlockLine* { return m_line; }
    auto index() const -> size_t { return m_index; }

private:
    const BlockLine* m_line{};
    size_t m_index{};
};

} // namespace parser
