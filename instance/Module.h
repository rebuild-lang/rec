#pragma once
#include "strings/String.h"
#include "strings/View.h"

#include "LocalScope.h"

namespace instance {

using Name = strings::String;

struct Module {
    Name name;
    LocalScope locals;
};
using ModuleView = const Module *;

inline auto nameOf(const Module &m) -> const Name & { return m.name; }

} // namespace instance
