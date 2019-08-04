#pragma once
#include "LocalScope.h"

#include "parser/Tree.h"

namespace instance {

using parser::Block;

struct Body {
    LocalScope locals{};
    Block block{};
};

} // namespace instance
