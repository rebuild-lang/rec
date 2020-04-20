#include "Function.h"

#include "Entry.h"

namespace instance {

auto Function::lookupParameter(NameView name) const -> OptParameterView {
    auto r = parameterScope.byName(name);
    if (!r.single()) return {};
    return r.frontValue().get(meta::type<VariablePtr>)->parameter;
}

} // namespace instance
