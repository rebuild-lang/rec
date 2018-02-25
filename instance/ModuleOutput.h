#pragma once
#include "Module.h"

#include "strings/Output.h"

namespace instance {

inline auto operator<<(std::ostream &out, const Module &m) -> std::ostream & { //
    return out << ":" << m.name;
}

} // namespace instance
