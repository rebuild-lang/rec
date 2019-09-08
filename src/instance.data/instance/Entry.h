#pragma once
#include "Function.h"
#include "Module.h"
#include "Type.h"
#include "Variable.h"

#include "meta/Variant.h"

namespace instance {

using EntryVariant = meta::Variant<Function, Variable, Parameter, Type, Module>;

// we need to inherit here, to allow forward declare this type
class Entry : public EntryVariant {
    using This = Entry;

public:
    META_VARIANT_CONSTRUCT(Entry, EntryVariant)
};
using EntryView = Entry*;

inline auto nameOf(const Entry& entry) -> NameView {
    return entry.visit([](const auto& i) -> decltype(auto) { return nameOf(i); });
}

} // namespace instance
