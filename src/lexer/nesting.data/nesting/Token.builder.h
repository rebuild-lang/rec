#pragma once

#include "Token.h"

namespace nesting {

using Tokens = std::vector<Token>;
using Column = text::Column;
using View = scanner::View;

namespace details {

struct IdentBuilder {
    using This = IdentBuilder;
    Token tok{};

    IdentBuilder() = delete;
    IdentBuilder(IdentifierLiteral) { tok = IdentifierLiteral{}; }
    IdentBuilder(OperatorLiteral) { tok = OperatorLiteral{}; }

    auto lit() & -> IdentifierLiteral& {
        if (tok.holds<OperatorLiteral>()) return tok.get<OperatorLiteral>();
        return tok.get<IdentifierLiteral>();
    }

    auto leftSeparated() && -> This {
        lit().value.leftSeparated = true;
        return *this;
    }
    auto rightSeparated() && -> This {
        lit().value.rightSeparated = true;
        return *this;
    }
    auto bothSeparated() && -> This {
        lit().value.leftSeparated = true;
        lit().value.rightSeparated = true;
        return *this;
    }

    template<size_t N>
    auto text(const char (&text)[N]) && -> This {
        lit().range.view = View{text};
        return *this;
    }

    auto id() && -> Token { return std::move(tok); }
};

template<class Tok>
struct TokenBuilder {
    static auto build(Tok&& t) -> Token { return {std::move(t)}; }
};
template<>
struct TokenBuilder<Token> {
    static auto build(Token&& t) -> Token { return std::move(t); }
};

template<>
struct TokenBuilder<IdentBuilder> {
    static auto build(IdentBuilder&& b) -> Token { return std::move(b).id(); }
};

} // namespace details

template<class Tok>
auto buildToken(Tok&& t) -> Token {
    return details::TokenBuilder<Tok>::build(std::forward<Tok>(t));
}

template<class... Tok>
auto buildTokens(Tok&&... t) -> Tokens {
    return Tokens{::nesting::buildToken(std::forward<Tok>(t))...};
}

template<size_t N>
auto id(const char (&text)[N]) -> details::IdentBuilder {
    return details::IdentBuilder(IdentifierLiteral{}).text(text);
}

template<size_t N>
auto op(const char (&text)[N]) -> details::IdentBuilder {
    return details::IdentBuilder(OperatorLiteral{}).text(text);
}

template<size_t N>
auto num(const char (&intPart)[N]) -> NumberLiteral {
    auto lit = NumberLiteral{};
    lit.value.integerPart += View{intPart};
    lit.value.radix = scanner::Radix::decimal;
    return lit;
}

inline auto blockStart(Column c) -> filter::Token {
    auto tok = filter::BlockStartIndentation{};
    tok.range.end.column = c;
    return tok;
}
inline auto blockEnd(Column c) -> filter::Token {
    auto tok = filter::BlockEndIndentation{};
    tok.range.begin.column = c;
    return tok;
}
inline auto newLine(Column c) -> filter::Token {
    auto tok = filter::NewLineIndentation{};
    tok.range.end.column = c;
    return tok;
}
inline auto colon() -> Token { return ColonSeparator{}; }

template<class... Lines>
auto blk(Column c, Lines&&... lines) -> Token {
    auto tok = BlockLiteral{{{std::forward<Lines>(lines)...}}, {}};
    tok.range.end.column = c;
    return tok;
}

} // namespace nesting
