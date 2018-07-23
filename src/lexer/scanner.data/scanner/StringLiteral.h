#pragma once
#include "strings/Rope.h"

namespace scanner {

using Rope = strings::Rope;

struct StringLiteralValue {
    using This = StringLiteralValue;
    Rope text;

    bool operator==(const This& o) const { return o.text == text; }
    bool operator!=(const This& o) const { return !(*this == o); }
};

} // namespace scanner
