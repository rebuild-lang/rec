#pragma once
#include "IdentifierLiteralValue.ostream.h"
#include "NewLineIndentationValue.ostream.h"
#include "NumberLiteralValue.ostream.h"
#include "StringLiteralValue.ostream.h"

#include "scanner/Token.h"

#include "text/Range.ostream.h"

namespace scanner {

inline auto operator<<(std::ostream& out, const WhiteSpaceSeparator&) -> std::ostream& { return out << "<space>"; }
inline auto operator<<(std::ostream& out, const NewLineIndentation& nli) -> std::ostream& {
    return out << "<\\n" << nli.value << ">";
}

inline auto operator<<(std::ostream& out, const CommentLiteral& c) -> std::ostream& {
    return out << "<##: " << c.input << ">";
}
inline auto operator<<(std::ostream& out, const ColonSeparator&) -> std::ostream& { return out << "<:>"; }
inline auto operator<<(std::ostream& out, const CommaSeparator&) -> std::ostream& { return out << "<,>"; }
inline auto operator<<(std::ostream& out, const SemicolonSeparator&) -> std::ostream& { return out << "<;>"; }
inline auto operator<<(std::ostream& out, const SquareBracketOpen&) -> std::ostream& { return out << "<[>"; }
inline auto operator<<(std::ostream& out, const SquareBracketClose&) -> std::ostream& { return out << "<]>"; }
inline auto operator<<(std::ostream& out, const BracketOpen&) -> std::ostream& { return out << "<(>"; }
inline auto operator<<(std::ostream& out, const BracketClose&) -> std::ostream& { return out << "<)>"; }

inline auto operator<<(std::ostream& out, const StringLiteral& str) -> std::ostream& {
    return out << "<str: " << str.value << ">";
}
inline auto operator<<(std::ostream& out, const NumberLiteral& num) -> std::ostream& {
    return out << "<num: " << num.value << ">";
}

inline auto operator<<(std::ostream& out, const IdentifierLiteral& id) -> std::ostream& {
    return out << "<id(" << id.value.type << "): " << id.input << ">";
}

inline auto operator<<(std::ostream& out, const InvalidEncoding& e) -> std::ostream& {
    return out << "<EncErr: " << e.input << ">";
}
inline auto operator<<(std::ostream& out, const UnexpectedCharacter& u) -> std::ostream& {
    return out << "<Unexp: " << u.input << ">";
}

inline auto operator<<(std::ostream& out, const Token& t) -> std::ostream& {
    return t.visit([&](const auto& v) -> decltype(auto) { return out << v; });
}

} // namespace scanner
