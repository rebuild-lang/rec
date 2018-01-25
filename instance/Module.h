#pragma once
#include "meta/Flags.h"
#include "strings/String.h"
#include "strings/View.h"

#include "LocalScope.h"

namespace instance {

using Name = strings::String;

enum class ModuleFlag {
    CompileTime = 1 << 0,
    RunTime = 1 << 1,
    Final = 1 << 2,
};
using ModuleFlags = meta::Flags<ModuleFlag>;

struct Module {
    Name name;
    ModuleFlags flags;
    LocalScope locals;
};
using ModuleView = const Module*;

inline auto nameOf(const Module& m) -> const Name& { return m.name; }

} // namespace instance
