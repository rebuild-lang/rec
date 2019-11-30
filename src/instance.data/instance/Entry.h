#pragma once
#include "Function.h"
#include "Module.h"
#include "Type.h"
#include "Variable.h"

#include "meta/Variant.h"

namespace instance {

using EntryVariant = meta::Variant<Function, Variable, Parameter, Type, Module>;

// we need to inherit here, to allow forward declare this type
struct Entry final : EntryVariant {
    META_VARIANT_CONSTRUCT(Entry, EntryVariant)
};
using EntryView = Entry*;

inline auto nameOf(const Entry& entry) -> NameView {
    return entry.visit([](const auto& i) -> decltype(auto) {
        // note: if an Entry member has no nameOf overload, this function becomes recursive
        static_assert(static_cast<NameView (*)(decltype(i))>(&nameOf));

        return nameOf(i);
    });
}

} // namespace instance
