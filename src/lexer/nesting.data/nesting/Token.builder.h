#pragma once

#include "Token.h"

namespace nesting {

using Tokens = std::vector<Token>;
using scanner::View;
using text::Column;

namespace details {

struct TokenLineBuilder {
    using This = TokenLineBuilder;
    BlockLine line{};

    template<class... Tok>
    auto tokens(Tok&&... t) && -> This {
        (line.tokens.push_back(std::forward<Tok>(t)), ...);
        return std::move(*this);
    }

    template<class... Tok>
    auto insignificants(Tok&&... t) && -> This {
        (line.insignificants.emplace_back(std::forward<Tok>(t)), ...);
        return std::move(*this);
    }

    auto build() && -> BlockLine { return std::move(line); }
};

template<class Tok>
struct TokenBuilder {
    static auto build(Tok&& t) -> Token { return {std::move(t)}; }
};
template<>
struct TokenBuilder<Token> {
    static auto build(Token&& t) -> Token { return std::move(t); }
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

inline auto id(View view) -> IdentifierLiteral { return IdentifierLiteral{view}; }

template<size_t N>
auto op(const char (&text)[N]) -> OperatorLiteral {
    return OperatorLiteral{View{text}};
}

template<size_t N>
auto num(const char (&intPart)[N]) -> NumberLiteral {
    auto lit = NumberLiteral{};
    lit.value.integerPart += View{intPart};
    lit.value.radix = scanner::Radix::decimal;
    return lit;
}

inline auto colon() -> Token { return ColonSeparator{}; }

inline auto line() -> details::TokenLineBuilder { return {}; }

template<class... Lines>
auto buildBlockLines(Lines&&... lines) -> BlockLines {
    return BlockLines{std::forward<Lines>(lines).build()...};
}

template<class... Lines>
auto blk(Lines&&... lines) -> Token {
    return BlockLiteral{{}, {buildBlockLines(std::forward<Lines>(lines)...)}};
}

} // namespace nesting
