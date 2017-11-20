#pragma once
#include "argument_builder.h"

#include "function.h"

namespace instance {

namespace details {

struct fun_builder {
    using this_t = fun_builder;
    function_t fun_;
    std::vector<arg_builder> args_;

    template<size_t N>
    fun_builder(const char (&name)[N]) {
        fun_.name = strings::utf8_string(name);
    }

    template<class... Argument>
    auto args(Argument &&... argument) && -> this_t {
        auto x = {(args_.push_back(std::forward<Argument>(argument)), 0)...};
        (void)x;
        return *this;
    }

    auto build(const scope_t &scope) && -> function_t {
        for (auto &&a : args_) fun_.arguments.push_back(std::move(a).build(scope));
        return std::move(fun_);
    }
};

} // namespace details

template<size_t N>
inline auto fun(const char (&name)[N]) -> details::fun_builder {
    return {name};
}

} // namespace instance
