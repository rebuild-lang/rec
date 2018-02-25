#pragma once
#include "LocalScope.h"

#include "expression/Tree.h"

namespace instance {

using Block = parser::expression::Block;

struct Body {
    LocalScope locals;
    Block block;
};

} // namespace instance
