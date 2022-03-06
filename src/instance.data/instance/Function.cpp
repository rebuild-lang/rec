#include "Function.h"

#include "Entry.h"

namespace instance {

auto Function::lookupParameter(NameView paramName) const -> OptParameterView {
    auto r = parameterScope.byName(paramName);
    if (!r.single()) return {};
    return r.frontValue().get(meta::type<VariablePtr>)->parameter;
}

} // namespace instance
