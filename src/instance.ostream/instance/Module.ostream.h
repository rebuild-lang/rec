#pragma once
#include "instance/Module.h"

#include "strings/String.stream.h"

namespace instance {

inline auto operator<<(std::ostream& out, const Module& m) -> std::ostream& { //
    return out << ":" << m.name;
}

inline auto operator<<(std::ostream& out, const ModulePtr& m) -> std::ostream& { return out << *m; }
inline auto operator<<(std::ostream& out, ModuleView m) -> std::ostream& { return out << *m; }

} // namespace instance
