#pragma once
#include "parser/Type.h"

#include "instance/Module.h"

#include "strings/String.ostream.h"

namespace parser {

inline auto operator<<(std::ostream& out, const Type& t) -> std::ostream& {
    return out << ":" << t.module->name << "(bytes: " << t.size << ")";
}

} // namespace parser
