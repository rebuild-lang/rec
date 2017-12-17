#pragma once
#include "ArgumentBuilder.h"

#include "Function.h"

namespace instance {

namespace details {

struct FunctionBuilder {
    using This = FunctionBuilder;
    Function fun_;
    std::vector<ArgumentBuilder> args_;

    template<size_t N>
    FunctionBuilder(const char (&name)[N]) {
        fun_.name = strings::String(name);
    }

    template<class... Argument>
    auto args(Argument &&... argument) && -> This {
        auto x = {(args_.push_back(std::forward<Argument>(argument)), 0)...};
        (void)x;
        return *this;
    }

    auto build(const Scope &scope) && -> Function {
        for (auto &&a : args_) fun_.arguments.push_back(std::move(a).build(scope));
        return std::move(fun_);
    }
};

} // namespace details

template<size_t N>
inline auto fun(const char (&name)[N]) -> details::FunctionBuilder {
    return {name};
}

} // namespace instance
