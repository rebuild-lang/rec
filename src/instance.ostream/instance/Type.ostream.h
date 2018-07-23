#pragma once
#include "instance/Type.h"

#include "strings/View.ostream.h"

namespace instance {

inline auto operator<<(std::ostream& out, const Type& t) -> std::ostream& {
    return out << ":" << t.name << "(bytes: " << t.size << ")";
}

} // namespace instance
