#pragma once
#include <strings/View.h>
#include <text/DecodedPosition.h>
#include <vector>

namespace scanner {

using strings::View;
using text::DecodedErrorPosition;

using OperatorWrongClose = text::InputPosition<struct OperatorWrongCloseTag>;
using OperatorUnexpectedClose = text::InputPosition<struct OperatorUnexpectedCloseTag>;
using OperatorNotClosed = text::InputPosition<struct OperatorNotClosedTag>;

using IdentifierLiteralError = meta::Variant< //
    DecodedErrorPosition,
    OperatorWrongClose, // opening and closing don't match
    OperatorUnexpectedClose, // missing opening
    OperatorNotClosed // missing closing
    >;
using IdentifierLiteralErrors = std::vector<IdentifierLiteralError>;

enum class IdentifierLiteralType : int {
    normal, // normal identifier
    member, // '.' before normal identifier
    pattern_placeholder, // '$' before normal identifier
    operator_sign, // contains special operator characters (may need splitting)
};
constexpr auto to_string(IdentifierLiteralType type) -> View {
    switch (type) {
    case IdentifierLiteralType::normal: return View{"n"};
    case IdentifierLiteralType::member: return View{"."};
    case IdentifierLiteralType::pattern_placeholder: return View{"$"};
    case IdentifierLiteralType::operator_sign: return View{"op"};
    }
    return View{"[corrupted-type]"};
}

struct IdentifierLiteralValue {
    using This = IdentifierLiteralValue;
    IdentifierLiteralType type{};
    IdentifierLiteralErrors errors{};

    auto hasErrors() const { return !errors.empty(); }

    bool operator==(const This& o) const noexcept { return errors == o.errors; }
    bool operator!=(const This& o) const noexcept { return !(*this == o); }
};

} // namespace scanner
