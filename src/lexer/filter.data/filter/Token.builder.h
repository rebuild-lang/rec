#pragma once
#include "Token.h"

#include "strings/View.h"

namespace filter {

using Tokens = std::vector<Token>;
using TokenLines = std::vector<TokenLine>;
using strings::View;

namespace details {

struct TokenLineBuilder {
    using This = TokenLineBuilder;
    TokenLine line{};

    template<class... Tok>
    auto tokens(Tok&&... t) && -> This {
        (line.tokens.push_back(std::forward<Tok>(t)), ...);
        return std::move(*this);
    }

    template<class... Tok>
    auto insignificants(Tok&&... t) && -> This {
        auto add = [this]<class TR>(TR&& t) {
            using T = std::remove_const_t<std::remove_reference_t<TR>>;
            if (std::is_same_v<T, NewLineIndentation>) line.newLineIndex = static_cast<int>(line.insignificants.size());
            if (std::is_same_v<T, BlockStartColon>)
                line.blockStartColonIndex = static_cast<int>(line.insignificants.size());
            if (std::is_same_v<T, BlockEndIdentifier>)
                line.blockEndIdentifierIndex = static_cast<int>(line.insignificants.size());
            line.insignificants.emplace_back(std::forward<TR>(t));
        };
        (add(std::forward<Tok>(t)), ...);
        return std::move(*this);
    }

    auto build() && -> TokenLine { return std::move(line); }
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

inline auto id() -> IdentifierLiteral { return IdentifierLiteral{}; }

inline auto id(View v) -> IdentifierLiteral { return IdentifierLiteral{{v}}; }

template<size_t N>
auto id(const char (&text)[N]) -> IdentifierLiteral {
    auto type = [&] {
        if (text[0] == '.') return scanner::IdentifierLiteralType::member;
        if (text[0] == '$') return scanner::IdentifierLiteralType::pattern_placeholder;
        return scanner::IdentifierLiteralType::normal;
    }();
    return IdentifierLiteral{View{text}, {type, {}}};
}

template<size_t N>
auto op(const char (&text)[N]) -> IdentifierLiteral {
    return IdentifierLiteral{
        {View{text}}, scanner::IdentifierLiteralValue{scanner::IdentifierLiteralType::operator_sign}};
}

template<class Tok>
auto buildToken(Tok&& t) -> Token {
    return details::TokenBuilder<Tok>::build(std::forward<Tok>(t));
}

template<class... Tok>
auto buildTokens(Tok&&... t) -> Tokens {
    return Tokens{::filter::buildToken(std::forward<Tok>(t))...};
}

inline auto newLine(uint32_t column = 1) -> NewLineIndentation {
    return NewLineIndentation{{}, {{}, scanner::Column{column}}};
}

inline auto blockEnd(View v) -> BlockEndIdentifier { return BlockEndIdentifier{{v}}; }

inline auto line() -> details::TokenLineBuilder { return {}; }

template<class... Lines>
auto buildTokenLines(Lines&&... l) -> TokenLines {
    return TokenLines{std::forward<Lines>(l).build()...};
}

} // namespace filter
