#pragma once
#include "Parameter.builder.h"

#include "Function.h"

namespace instance {

namespace details {

struct FunctionBuilder {
    using This = FunctionBuilder;
    FunctionPtr fun_{};
    std::vector<ParameterBuilder> params_{};

    [[nodiscard]] FunctionBuilder()
        : fun_(std::make_shared<Function>()) {}

    template<size_t N>
    [[nodiscard]] explicit FunctionBuilder(const char (&name)[N])
        : FunctionBuilder() {
        fun_->name = Name{name};
    }

    [[nodiscard]] auto runtime() && -> This {
        fun_->flags |= FunctionFlag::run_time;
        return std::move(*this);
    }

    [[nodiscard]] auto compiletime() && -> This {
        fun_->flags |= FunctionFlag::compile_time;
        return std::move(*this);
    }

    [[nodiscard]] auto compiletime_sideeffects() && -> This {
        fun_->flags |= FunctionFlags(FunctionFlag::compile_time, FunctionFlag::compile_time_side_effects);
        return std::move(*this);
    }

    template<class... Parameter>
    [[nodiscard]] auto params(Parameter&&... parameter) && -> This {
        params_.insert(params_.end(), {std::forward<Parameter>(parameter)...});
        return std::move(*this);
    }

    [[nodiscard]] auto rawIntrinsic(void (*f)(uint8_t*, intrinsic::ContextInterface*)) && -> This {
        fun_->body = instance::IntrinsicCall{f};
        return std::move(*this);
    }

    [[nodiscard]] auto build(const Scope& scope) && -> FunctionPtr {
        for (auto&& a : params_) fun_->parameters.emplace_back(std::move(a).build(scope, fun_->parameterScope));
        return std::move(fun_);
    }
};

} // namespace details

template<size_t N>
inline auto fun(const char (&name)[N]) {
    return details::FunctionBuilder{name};
}

} // namespace instance
