#pragma once
#include <meta/Variant.h>
#include <text/DecodedPosition.h>
#include <text/Position.h>

namespace scanner {

using text::Column;
using text::DecodedErrorPosition;

using MixedIndentCharacter = text::InputPosition<struct MixedIndentCharacterTag>;

using NewLineIndentError = meta::Variant<DecodedErrorPosition, MixedIndentCharacter>;
using NewLineIndentErrors = std::vector<NewLineIndentError>;

struct NewLineIndentationValue {
    using This = NewLineIndentationValue;
    NewLineIndentErrors errors{};
    Column indentColumn{};

    auto hasErrors() const { return !errors.empty(); }

    bool operator==(const This& o) const noexcept { return errors == o.errors && indentColumn == o.indentColumn; }
    bool operator!=(const This& o) const noexcept { return !(*this == o); }
};

} // namespace scanner
