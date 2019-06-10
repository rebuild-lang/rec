#pragma once
#include "instance/Module.h"

#include "strings/String.stream.h"

namespace instance {

inline auto operator<<(std::ostream& out, const Module& m) -> std::ostream& { //
    return out << ":" << m.name;
}

} // namespace instance
