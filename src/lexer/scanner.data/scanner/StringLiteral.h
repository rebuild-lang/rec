#pragma once
#include "strings/Rope.h"

#include "text/Position.h"

#include <vector>

namespace scanner {

using Rope = strings::Rope;

struct StringError {
    using This = StringError;

    enum class Kind {
        EndOfInput, // missing end quote
        InvalidEncoding, // could not decode UTF8 char
        InvalidEscape, // escape sequence unknown
        InvalidControl, // control character
        InvalidDecimalUnicode, // \u not valid
        InvalidHexUnicode, // \x not valid
    };

    Kind kind{};
    strings::View input{};
    text::Position position{};

    bool operator==(const This& o) const { return o.kind == kind && o.input == input && o.position == position; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
using StringErrors = std::vector<StringError>;

struct StringLiteralValue {
    using This = StringLiteralValue;
    Rope text;
    StringErrors errors;

    bool operator==(const This& o) const { return o.text == text && o.errors == errors; }
    bool operator!=(const This& o) const { return !(*this == o); }
};

} // namespace scanner
