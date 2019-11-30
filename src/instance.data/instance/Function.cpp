#include "Function.h"

#include "Entry.h"

namespace instance {

auto Function::lookupParameter(NameView name) const -> OptParameterView {
    auto r = parameterScope[name];
    if (!r.single()) return {};
    return &r.frontValue().get(meta::type<Parameter>);
}

} // namespace instance
