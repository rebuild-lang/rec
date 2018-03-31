#pragma once
#include "strings/Rope.h"

namespace scanner {

using Rope = strings::Rope;

struct StringLiteral {
    Rope text;

    bool operator==(const StringLiteral& o) const { return o.text == text; }
    bool operator!=(const StringLiteral& o) const { return !(*this == o); }
};

} // namespace scanner
