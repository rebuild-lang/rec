#pragma once
#include "strings/utf8_string.h"
#include "strings/utf8_view.h"

#include "local_scope.h"

namespace instance {

using name_t = strings::utf8_string;

struct module_t {
    name_t name;
    local_scope_t locals;
};
using module_ptr = const module_t *;

inline auto name_of(const module_t &m) -> const name_t & { return m.name; }

inline auto operator<<(std::ostream &out, const module_t &m) -> std::ostream & { return out << ":" << m.name; }

} // namespace instance
