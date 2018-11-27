#pragma once
#include "Token.h"

namespace filter {

using Tokens = std::vector<Token>;

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
        // lit().range.view = View{text};
        return *this;
    }

    auto id() && -> Token&& { return std::move(tok); }
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

inline auto id() -> details::IdentBuilder { return {IdentifierLiteral{}}; }

template<size_t N>
auto id(const char (&text)[N]) -> details::IdentBuilder {
    return details::IdentBuilder(IdentifierLiteral{}).text(text);
}

template<size_t N>
auto op(const char (&text)[N]) -> details::IdentBuilder {
    return details::IdentBuilder(OperatorLiteral{}).text(text);
}

template<class Tok>
auto buildToken(Tok&& t) -> Token {
    return details::TokenBuilder<Tok>::build(std::forward<Tok>(t));
}

template<class... Tok>
auto buildTokens(Tok&&... t) -> Tokens {
    return Tokens{::filter::buildToken(std::forward<Tok>(t))...};
}

} // namespace filter
