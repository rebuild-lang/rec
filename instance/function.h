#pragma once
#include "argument.h"

#include "meta/algorithm.h"
#include "strings/utf8_view.h"

namespace instance {

using view_t = strings::utf8_view;

enum class function_flag {
    compile_time = 1 << 0,
    run_time = 1 << 1,
};
struct function_t {
    name_t name;
    function_flag flags;
    argument_vec arguments;
    // precedence_level level;
    // block body;

    auto lookup_argument(const view_t &name) const -> decltype(auto) {
        return meta::find_if(arguments, [&](const auto &a) { return name.content_equals(a.name); });
    }
};
using function_ptr = const function_t *;

inline auto name_of(const function_t &fun) -> const name_t & { return fun.name; }

inline auto operator<<(std::ostream &out, const function_t &f) -> std::ostream & {
    // TODO: print flags
    return out << "fn " << f.name << '(' << f.arguments << ')';
}

} // namespace instance
