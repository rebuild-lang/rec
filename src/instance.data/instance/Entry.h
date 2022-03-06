#pragma once
#include "Function.h"
#include "Module.h"
#include "Type.h"
#include "Variable.h"

#include "meta/Variant.h"

namespace instance {

using EntryVariant = meta::Variant<FunctionPtr, VariablePtr, TypePtr, ModulePtr>;

// we need to inherit here, to allow forward declare this type
struct Entry final : EntryVariant {
    META_VARIANT_CONSTRUCT(Entry, EntryVariant)
};

} // namespace instance
