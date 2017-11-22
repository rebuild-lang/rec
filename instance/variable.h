#pragma once
#include "type.h"

#include "meta/flags.h"
#include "strings/join.h"

namespace instance {

enum class variable_flag {
    immutable = 1 << 0,
    alias = 1 << 1,
    compile_time = 1 << 2,
    run_time = 1 << 3,
};
using variable_flags = meta::flags<variable_flag>;

struct variable_t {
    name_t name;
    type_ptr type{};
    variable_flag flags{};
};
using variable_ptr = const variable_t *;

inline auto name_of(const variable_t &var) -> const name_t & { return var.name; }

inline auto operator<<(std::ostream &out, const variable_flags &f) -> std::ostream & {
    out << "flags=[";
    auto labels = std::vector<const char *>{};
    if (f.has_any(variable_flag::immutable)) labels.push_back("immutable");
    if (f.has_any(variable_flag::alias)) labels.push_back("alias");
    if (f.has_any(variable_flag::compile_time)) labels.push_back("compile_time");
    if (f.has_any(variable_flag::run_time)) labels.push_back("run_time");
    strings::join(out, labels, ", ");
    return out << ']';
}

inline auto operator<<(std::ostream &out, const variable_t &v) -> std::ostream & {
    // TODO: print flags
    return out << "var " << v.name << v.type;
}

} // namespace instance

META_FLAGS_OP(instance::variable_flag)
