#pragma once
#include "type.h"

#include "meta/flags.h"
#include "strings/join.h"
#include "strings/utf8_string.h"

namespace instance {

enum class argument_side { left, right, result };
enum class argument_flag {
    optional = 1 << 0,
    splatted = 1 << 1, // array type is gathered during call
    token = 1 << 5, // unparsed token
    expression = 1 << 6, // unevaluated expression
    compile_time = 1 << 7, // compile time result
    // …
};
using argument_flags = meta::flags<argument_flag>;

struct argument_t {
    name_t name;
    type_ptr type{};
    argument_side side{};
    argument_flags flags{};
};
using argument_ptr = const argument_t *;
using argument_vec = std::vector<argument_t>;

inline auto name_of(const argument_t &arg) -> const name_t & { return arg.name; }

inline auto operator<<(std::ostream &out, const argument_flags &f) -> std::ostream & {
    out << "flags=[";
    auto labels = std::vector<const char *>{};
    if (f.has_any(argument_flag::optional)) labels.push_back("optional");
    if (f.has_any(argument_flag::splatted)) labels.push_back("splatted");
    if (f.has_any(argument_flag::token)) labels.push_back("token");
    if (f.has_any(argument_flag::expression)) labels.push_back("expression");
    if (f.has_any(argument_flag::compile_time)) labels.push_back("compilte_time");
    strings::join(out, labels, ", ");
    return out << ']';
}

inline auto operator<<(std::ostream &out, const argument_t &a) -> std::ostream & {
    return out << a.name << a.type << ' ' << a.flags;
}

inline auto operator<<(std::ostream &out, const argument_vec &av) -> std::ostream & {
    strings::join(out, av, ", ");
    return out;
}

} // namespace instance

META_FLAGS_OP(instance::argument_flag);
