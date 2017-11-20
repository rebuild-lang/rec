#pragma once
#include "argument.h"

#include "scope.h"
#include "scope_lookup.h"

namespace instance {

namespace details {

struct arg_builder {
    using this_t = arg_builder;
    argument_t arg;
    name_t type_name;

    template<size_t N>
    arg_builder(const char (&name)[N]) {
        arg.name = strings::utf8_string(name);
    }

    arg_builder(const this_t &) = default;
    arg_builder(this_t &&) = default;
    this_t &operator=(const this_t &) = default;
    this_t &operator=(this_t &&) = default;

    template<size_t N>
    auto type(const char (&type)[N]) -> arg_builder {
        type_name = strings::utf8_string(type);
        return *this;
    }

    auto left() -> arg_builder {
        arg.side = argument_side::left;
        return *this;
    }
    auto right() -> arg_builder {
        arg.side = argument_side::right;
        return *this;
    }
    auto result() -> arg_builder {
        arg.side = argument_side::result;
        return *this;
    }
    auto optional() -> arg_builder {
        // arg.flags |= argument_flag::optional;
        return *this;
    }

    auto build(const scope_t &scope) && -> argument_t {
        if (!type_name.is_empty()) {
            arg.type = &lookup_a<type_t>(scope, type_name);
        }
        return arg;
    }
};

} // namespace details

template<size_t N>
inline auto arg(const char (&name)[N]) -> details::arg_builder {
    return {name};
}

} // namespace instance
