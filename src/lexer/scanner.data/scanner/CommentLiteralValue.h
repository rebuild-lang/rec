#pragma once
#include <text/DecodedPosition.h>

namespace scanner {

using text::DecodedErrorPosition;
using DecodedErrorPositions = std::vector<DecodedErrorPosition>;

struct CommentLiteralValue {
    using This = CommentLiteralValue;
    DecodedErrorPositions decodeErrors{};

    explicit operator bool() const { return decodeErrors.empty(); }

    bool operator==(const This& o) const noexcept { return decodeErrors == o.decodeErrors; }
    bool operator!=(const This& o) const noexcept { return !(*this == o); }
};

} // namespace scanner
