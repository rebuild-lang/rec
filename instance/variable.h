#pragma once
#include "type.h"

namespace instance {

enum class variable_flag {
    immutable = 1 << 0,
    alias = 1 << 1,
    compile_time = 1 << 2,
    run_time = 1 << 3,
};
struct variable_t {
    name_t name;
    type_ptr type{};
    variable_flag flags{};
};
using variable_ptr = const variable_t *;

inline auto name_of(const variable_t &var) -> const name_t & { return var.name; }

inline auto operator<<(std::ostream &out, const variable_t &v) -> std::ostream & {
    // TODO: print flags
    return out << "var " << v.name << v.type;
}

} // namespace instance
