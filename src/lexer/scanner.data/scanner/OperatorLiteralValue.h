#pragma once
#include <text/DecodedPosition.h>

#include <vector>

namespace scanner {

using text::DecodedErrorPosition;

using OperatorWrongClose = text::InputPosition<struct OperatorWrongCloseTag>;
using OperatorUnexpectedClose = text::InputPosition<struct OperatorUnexpectedCloseTag>;
using OperatorNotClosed = text::InputPosition<struct OperatorNotClosedTag>;

using OperatorLiteralError = meta::Variant< //
    DecodedErrorPosition,
    OperatorWrongClose,
    OperatorUnexpectedClose,
    OperatorNotClosed>;
using OperatorLiteralErrors = std::vector<OperatorLiteralError>;

struct OperatorLiteralValue {
    using This = OperatorLiteralValue;
    OperatorLiteralErrors errors{};

    auto hasErrors() const { return !errors.empty(); }

    bool operator==(const This& o) const noexcept { return errors == o.errors; }
    bool operator!=(const This& o) const noexcept { return !(*this == o); }
};

} // namespace scanner
