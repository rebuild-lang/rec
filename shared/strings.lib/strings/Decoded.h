#pragma once
#include <meta/Variant.h>

#include "CodePoint.h"
#include "View.h"

namespace strings {

struct DecodedError {
    using This = DecodedError;
    View input;

    constexpr bool operator==(const This& o) const = default;
};
struct DecodedCodePoint {
    using This = DecodedCodePoint;
    View input;
    CodePoint cp;

    constexpr bool operator==(const This& o) const = default;
};

using Decoded = meta::Variant<DecodedCodePoint, DecodedError>;

} // namespace strings
