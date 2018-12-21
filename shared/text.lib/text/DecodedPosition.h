#pragma once
#include <meta/Variant.h>

#include <strings/CodePoint.h>
#include <strings/View.h>

#include "Position.h"

namespace text {

using strings::CodePoint;
using strings::View;

template<class...>
struct InputPosition {
    using This = InputPosition;
    View input;
    Position position;

    constexpr bool operator==(const This& o) const { return input == o.input && position == o.position; }
    constexpr bool operator!=(const This& o) const { return !(*this == o); }
};

struct CodePointPosition {
    using This = CodePointPosition;
    View input;
    Position position;
    CodePoint codePoint;

    constexpr bool operator==(const This& o) const {
        return input == o.input && codePoint == o.codePoint && position == o.position;
    }
    constexpr bool operator!=(const This& o) const { return !(*this == o); }
};

using NewlinePosition = InputPosition<struct NewlinePositionTag>;
using DecodedErrorPosition = InputPosition<struct DecodedErrorPositionTag>;

using DecodedPosition = meta::Variant<CodePointPosition, NewlinePosition, DecodedErrorPosition>;

} // namespace text
