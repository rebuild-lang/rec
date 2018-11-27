#pragma once
#include <meta/Variant.h>

#include <strings/CodePoint.h>
#include <strings/View.h>

#include "Position.h"

namespace text {

using strings::CodePoint;
using strings::View;

struct CodePointPosition {
    using This = CodePointPosition;
    View input;
    CodePoint codePoint;
    Position position;

    constexpr bool operator==(const This& o) const {
        return input == o.input && codePoint == o.codePoint && position == o.position;
    }
    constexpr bool operator!=(const This& o) const { return !(*this == o); }
};
struct NewlinePosition {
    using This = NewlinePosition;
    View input;
    Position position;

    constexpr bool operator==(const This& o) const { return input == o.input && position == o.position; }
    constexpr bool operator!=(const This& o) const { return !(*this == o); }
};
struct DecodedErrorPosition {
    using This = DecodedErrorPosition;
    View input;
    Position position;

    constexpr bool operator==(const This& o) const { return input == o.input && position == o.position; }
    constexpr bool operator!=(const This& o) const { return !(*this == o); }
};

using DecodedPosition = meta::Variant<CodePointPosition, NewlinePosition, DecodedErrorPosition>;

} // namespace text
