#pragma once
#include "LocalScope.h"

#include "parser/Expression.h"

namespace instance {

using parser::Block;

/// function body is implemented in C++ as part of the Intrinsic API
struct IntrinsicCall {
    using Exec = void (*)(uint8_t*, intrinsic::ContextInterface*);
    Exec exec;
};

/// function body is implemented by the user defined block
struct ParsedBlock {
    LocalScope locals{};
    Block block{};
};

using Body = meta::Variant<ParsedBlock, IntrinsicCall>;

} // namespace instance
