#pragma once
#include "Parameter.builder.h"

#include "Function.h"

namespace instance {

namespace details {

struct FunctionBuilder {
    using This = FunctionBuilder;
    Function fun_;
    std::vector<ParameterBuilder> params_;

    template<size_t N>
    explicit FunctionBuilder(const char (&name)[N]) {
        fun_.name = Name{name};
    }

    auto runtime() && -> This {
        fun_.flags |= FunctionFlag::runtime;
        return std::move(*this);
    }

    auto compiletime() && -> This {
        fun_.flags |= FunctionFlag::compiletime;
        return std::move(*this);
    }

    auto compiletime_sideeffects() && -> This {
        fun_.flags |= FunctionFlags(FunctionFlag::compiletime, FunctionFlag::compiletime_sideeffects);
        return std::move(*this);
    }

    template<class... Parameter>
    auto params(Parameter&&... parameter) && -> This {
        auto x = {(params_.push_back(std::forward<Parameter>(parameter)), 0)...};
        (void)x;
        return std::move(*this);
    }

    auto rawIntrinsic(void (*f)(uint8_t*, intrinsic::Context*)) && -> This {
        fun_.body.block.expressions.emplace_back(parser::IntrinsicCall{f});
        return std::move(*this);
    }

    auto build(const Scope& scope) && -> Function {
        for (auto&& a : params_) fun_.parameters.emplace_back(std::move(a).build(scope, fun_.parameterScope));
        return std::move(fun_);
    }
};

} // namespace details

template<size_t N>
inline auto fun(const char (&name)[N]) {
    return details::FunctionBuilder{name};
}

} // namespace instance
