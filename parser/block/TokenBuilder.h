#pragma once

#include "parser/block/Token.h"

namespace parser::block {

using Tokens = std::vector<Token>;
using Column = scanner::Column;
using View = scanner::View;

namespace details {

struct IdentBuilder {
    using This = IdentBuilder;
    Token tok{};

    IdentBuilder() = delete;
    IdentBuilder(IdentifierLiteral) { tok.data = IdentifierLiteral{}; }
    IdentBuilder(OperatorLiteral) { tok.data = OperatorLiteral{}; }

    auto lit() & -> IdentifierLiteral & {
        if (tok.oneOf<OperatorLiteral>()) return tok.data.get<OperatorLiteral>();
        return tok.data.get<IdentifierLiteral>();
    }

    auto leftSeparated() && -> This {
        lit().leftSeparated = true;
        return *this;
    }
    auto rightSeparated() && -> This {
        lit().rightSeparated = true;
        return *this;
    }
    auto bothSeparated() && -> This {
        lit().leftSeparated = true;
        lit().rightSeparated = true;
        return *this;
    }

    template<size_t N>
    auto text(const char (&text)[N]) && -> This {
        tok.range.text = View{text};
        return *this;
    }

    auto id() && -> Token { return std::move(tok); }
};

template<class Tok>
struct TokenBuilder {
    static auto build(Tok &&t) -> Token {
        auto tok = Token{};
        tok.data = std::move(t);
        return tok;
    }
};
template<>
struct TokenBuilder<Token> {
    static auto build(Token &&t) -> Token { return std::move(t); }
};

template<>
struct TokenBuilder<IdentBuilder> {
    static auto build(IdentBuilder &&b) -> Token { return std::move(b).id(); }
};

} // namespace details

template<class Tok>
auto buildToken(Tok &&t) -> Token {
    return details::TokenBuilder<Tok>::build(std::forward<Tok>(t));
}

template<class... Tok>
auto buildTokens(Tok &&... t) -> Tokens {
    return Tokens{::parser::block::buildToken(std::forward<Tok>(t))...};
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
    lit.integerPart += View{intPart};
    lit.radix = scanner::Radix::decimal;
    return lit;
}

inline auto blockStart(Column c) -> filter::Token {
    auto tok = filter::Token{};
    tok.range.end.column = c;
    tok.data = filter::BlockStartIndentation{};
    return tok;
}
inline auto blockEnd(Column c) -> filter::Token {
    auto tok = filter::Token{};
    tok.range.end.column = c;
    tok.data = filter::BlockEndIndentation{};
    return tok;
}
inline auto newLine(Column c) -> filter::Token {
    auto tok = filter::Token{};
    tok.range.end.column = c;
    tok.data = filter::NewLineIndentation{};
    return tok;
}

template<class... Lines>
auto blk(Column c, Lines &&... lines) -> Token {
    auto tok = Token{};
    tok.range.end.column = c;
    tok.data = BlockLiteral{{std::forward<Lines>(lines)...}};
    return tok;
}

} // namespace parser::block
