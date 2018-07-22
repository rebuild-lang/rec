#pragma once
#include "Function.h"
#include "Module.h"
#include "Type.h"
#include "Variable.h"

#include "meta/Variant.h"

namespace instance {

using NodeVariant = meta::Variant<Function, Variable, Argument, Type, Module>;

// we need to inherit here, to allow forward declare this type
class Node : public NodeVariant {
    using This = Node;

public:
    META_VARIANT_CONSTRUCT(Node, NodeVariant)
};
using NodeView = Node*;

inline auto nameOf(const Node& v) -> const Name& {
    return v.visit([](const auto& i) -> decltype(auto) { return nameOf(i); });
}

} // namespace instance
