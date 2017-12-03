#pragma once
#include "function.h"
#include "module.h"
#include "type.h"
#include "variable.h"

#include "meta/variant.h"

namespace instance {

using node_variant = meta::variant<function_t, variable_t, argument_t, type_t, module_t>;

// we need to inherit here, to allow forward declare this type
class node_t : public node_variant {
    using this_t = node_t;

public:
    META_VARIANT_CONSTRUCT(node_t, node_variant)
};

inline auto name_of(const node_t &v) -> const name_t & {
    return v.visit([](const auto &i) -> decltype(auto) { return name_of(i); });
}

} // namespace instance
