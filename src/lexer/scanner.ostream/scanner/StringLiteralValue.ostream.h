#pragma once
#include <scanner/StringLiteralValue.h>

#include <strings/View.ostream.h>
#include <text/Position.ostream.h>

#include <ostream>

namespace scanner {

inline auto operator<<(std::ostream& out, const StringError::Kind& errorKind) -> std::ostream& {
    switch (errorKind) {
    case StringError::Kind::EndOfInput: return out << "EndOfInput";
    case StringError::Kind::InvalidEncoding: return out << "InvalidEncoding";
    case StringError::Kind::InvalidEscape: return out << "InvalidEscape";
    case StringError::Kind::InvalidControl: return out << "InvalidControl";
    case StringError::Kind::InvalidDecimalUnicode: return out << "InvalidDecimalUnicode";
    case StringError::Kind::InvalidHexUnicode: return out << "InvalidHexUnicode";
    }
    return out << "???";
}

inline auto operator<<(std::ostream& out, const StringError& error) -> std::ostream& {
    return out << error.kind << " \"" << error.input << "\" at " << error.position;
}

auto operator<<(std::ostream& out, const StringErrors& errors) -> std::ostream&;

auto operator<<(std::ostream& out, const StringLiteralValue& lit) -> std::ostream&;

} // namespace scanner
