#pragma once
#include <meta/Variant.h>

#include "CodePoint.h"
#include "View.h"

namespace strings {

struct DecodedError {
    using This = DecodedError;
    View input;

    constexpr bool operator==(const This& o) const { return input == o.input; }
    constexpr bool operator!=(const This& o) const { return !(*this == o); }
};
struct DecodedCodePoint {
    using This = DecodedCodePoint;
    View input;
    CodePoint cp;

    constexpr bool operator==(const This& o) const { return input == o.input && cp == o.cp; }
    constexpr bool operator!=(const This& o) const { return !(*this == o); }
};

using Decoded = meta::Variant<DecodedCodePoint, DecodedError>;

} // namespace strings
