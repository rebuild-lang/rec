#pragma once
#include "strings/utf8_string.h"

#include "type.h"

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
struct argument_t {
    name_t name;
    type_ptr type{};
    argument_side side{};
    argument_flag flags{};
};
using argument_ptr = const argument_t *;
using argument_vec = std::vector<argument_t>;

inline auto name_of(const argument_t &arg) -> const name_t & { return arg.name; }

inline auto operator<<(std::ostream &out, const argument_t &a) -> std::ostream & {
    // TODO: print flags
    return out << a.name << a.type;
}

inline auto operator<<(std::ostream &out, const argument_vec &av) -> std::ostream & {
    size_t i = 0, l = av.size();
    for (const auto &a : av) {
        out << a;
        if (++i != l) out << ", ";
    }
    return out;
}

} // namespace instance
