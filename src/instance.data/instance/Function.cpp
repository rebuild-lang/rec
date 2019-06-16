#include "Function.h"

#include "Node.h"

namespace instance {

auto Function::lookupParameter(NameView name) const -> OptParameterView {
    return parameterScope[name].map(
        [](const auto node) -> OptParameterView { return &node->template get<Parameter>(); });
}

} // namespace instance
