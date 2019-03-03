#pragma once
#include <meta/Variant.h>

#include <strings/CodePoint.h>
#include <strings/View.h>

#include "Position.h"

namespace text {

using strings::CodePoint;
using strings::View;

struct InputPositionData {
    using This = InputPositionData;
    View input{};
    Position position{};

    constexpr bool operator==(const This& o) const { return input == o.input && position == o.position; }
    constexpr bool operator!=(const This& o) const { return !(*this == o); }
};

template<class...>
struct InputPosition : InputPositionData {};

struct CodePointPosition : InputPositionData {
    using This = CodePointPosition;
    CodePoint codePoint{};
    Position endPosition{};

    constexpr bool operator==(const This& o) const {
        return input == o.input && position == o.position && codePoint == o.codePoint && endPosition == o.endPosition;
    }
    constexpr bool operator!=(const This& o) const { return !(*this == o); }
};

using NewlinePosition = InputPosition<struct NewlinePositionTag>;
using DecodedErrorPosition = InputPosition<struct DecodedErrorPositionTag>;

using DecodedPosition = meta::Variant<CodePointPosition, NewlinePosition, DecodedErrorPosition>;

} // namespace text
