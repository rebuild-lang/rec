#pragma once
#include "intrinsic/Function.h"
#include "intrinsic/Module.h"
#include "intrinsic/Parameter.h"
#include "intrinsic/Type.h"

#include "instance/Function.h"
#include "instance/IntrinsicContext.h"
#include "instance/Module.h"
#include "instance/Node.h"
#include "instance/Parameter.h"
#include "instance/Scope.h"
#include "instance/Type.h"

#include "meta/TypeList.h"

#include <cassert>

namespace intrinsicAdapter {

namespace details {

template<size_t U, size_t... V, size_t... I>
constexpr auto sumN(std::index_sequence<V...>, std::index_sequence<I...>) -> size_t {
    return ((I < U ? V : 0) + ... + 0);
}

template<size_t... V, size_t... I>
constexpr auto partialSum(std::index_sequence<V...> values, std::index_sequence<I...> indices) {
    return std::index_sequence<sumN<I>(decltype(values){}, decltype(indices){})...>{};
}

template<class Param>
struct ArgumentAt {
    static auto from(uint8_t* memory) -> Param { return *std::launder(reinterpret_cast<Param*>(memory)); }
};

template<class Param>
struct ArgumentAt<Param&> {
    static auto from(uint8_t* memory) -> Param& { return **std::launder(reinterpret_cast<Param**>(memory)); }
};

template<class Param>
struct ArgumentAt<const Param&> {
    static auto from(uint8_t* memory) -> Param& { return **std::launder(reinterpret_cast<Param**>(memory)); }
};

template<class Type>
constexpr auto argumentSize() -> size_t {
    using namespace intrinsic;
    if constexpr (Parameter<Type>::info().side == ParameterSide::Implicit) {
        return {};
    }
    else if constexpr (Parameter<Type>::info().flags.any(ParameterFlag::Assignable, ParameterFlag::Reference)) {
        return sizeof(void*);
    }
    else {
        return Parameter<Type>::typeInfo().size;
    }
}

template<auto* F, class... ParameterTypes>
struct Call {
    using Func = void (*)(ParameterTypes...);
    using Sizes = std::index_sequence<parameterSize<ParameterTypes>()...>;
    using Indices = std::make_index_sequence<sizeof...(ParameterTypes)>;

    static void call(uint8_t* memory, intrinsic::Context* context) {
        constexpr auto sizes = Sizes{};
        constexpr auto indices = Indices{};
        constexpr auto offsets = partialSum(sizes, indices);
        callImpl(memory, context, offsets);
    }

private:
    template<size_t... Offset>
    static void callImpl(uint8_t* memory, intrinsic::Context* context, std::index_sequence<Offset...>) {
        auto f = reinterpret_cast<Func>(F);
        f(argumentAt<ParameterTypes, Offset>(memory, context)...);
    }

    template<class Type, size_t Offset>
    static auto argumentAt(uint8_t* memory, intrinsic::Context* context) -> Type {
        using namespace intrinsic;
        if constexpr (Parameter<Type>::info().side == ParameterSide::Implicit) {
            return {context};
        }
        else {
            return ArgumentAt<Type>::from(memory + Offset);
        }
    }
};

} // namespace details

/*
 * intrinsic => instance adapter
 */
struct Adapter {
    using This = Adapter;

    template<class T>
    static auto moduleInstance() -> instance::Module {
        auto types = Types{};
        auto inner = This{&types};

        constexpr auto info = T::info();
        inner.moduleName(info.name);
        T::module(inner);

        inner.resolveTypes();
        return std::move(inner.instanceModule);
    }

    template<class T>
    void type() {
        using namespace intrinsic;
        constexpr auto info = TypeOf<T>::info();
        if constexpr (info.flags.any(TypeFlag::Instance)) {
            // TODO(arBmind)
        }
        else if constexpr (info.flags.any(TypeFlag::Construct)) {
            constructedType<T>(&TypeOf<T>::construct);
        }
        else {
            auto a = This{&types};
            a.moduleName(info.name);

            TypeOf<T>::module(a);

            a.instanceModule.locals.emplace([]() -> instance::Type {
                constexpr auto info = TypeOf<T>::info();
                auto r = instance::Type{};
                r.size = info.size;
                r.flags = typeFlags(info.flags);
                r.parser = typeParser(info.parser);
                r.clone = [](uint8_t* dest, const uint8_t* source) {
                    new (dest) T(*reinterpret_cast<const T*>(source));
                };
                r.makeUninitialized = [](const parser::TypeExpression& typeExpr) {
                    return parser::Value::uninitialized<T>(typeExpr);
                };
                return r;
            }());

            auto typeRange = a.instanceModule.locals[instance::Name{"type"}];
            assert(typeRange.single());
            types.map[info.name.data()] = &typeRange.frontValue().get<instance::Type>();

            instanceModule.locals.emplace(std::move(a.instanceModule));

            auto modRange = instanceModule.locals[info.name];
            assert(modRange.single());
            typeRange.frontValue().get<instance::Type>().module =
                &modRange.frontValue().get(meta::type<instance::Module>);
        }
    }

    template<class T>
    void module() {
        constexpr auto info = T::info();
        auto inner = This{&types};
        inner.moduleName(info.name);
        T::module(inner);

        instanceModule.locals.emplace(std::move(inner.instanceModule));
    }

    using FunctionInfoFunc = intrinsic::FunctionInfoFunc;

    template<auto* F, FunctionInfoFunc Info>
    void function() {
        return functionImpl<Info, F>(makeSignature(F));
    }

    struct IsImplicit {
        template<class Param>
        constexpr bool operator()(meta::Type<Param>) const {
            using namespace intrinsic;
            return Parameter<Param>::info().side == ParameterSide::Implicit;
        }
    };

    template<FunctionInfoFunc Info, auto* F, class... Params>
    void functionImpl(intrinsic::FunctionSignature<void, Params...>) {
        auto externParams = meta::TypeList<Params...>::template filterPred<IsImplicit>();
        functionImpl2<Info, F, Params...>(externParams);
    }

    template<FunctionInfoFunc Info, auto* F, class... Params, class... ExternParams>
    void functionImpl2(meta::TypeList<ExternParams...>) {
        // assert((GenericFunc)f2 == (GenericFunc)F);
        auto info = Info();
        auto r = instance::Function{};
        r.name = strings::to_string(info.name);
        r.flags = functionFlags(info.flags);
        r.parameters = instance::ParameterViews{parameter<ExternParams>(r.parameterScope)...};

        auto call = &details::Call<F, Params...>::call;
        r.body.block.nodes.emplace_back(parser::IntrinsicCall{call});

        auto indices = std::make_index_sequence<sizeof...(ExternParams)>{};
        trackParameters<ExternParams...>(r.parameters, indices);

        instanceModule.locals.emplace(std::move(r));
    }

    void moduleName(intrinsic::Name name) { instanceModule.name = strings::to_string(name); }

private:
    struct ParameterRef {
        instance::Parameter* ref;
        const char* typeName;
    };
    using Parameters = std::vector<ParameterRef>;
    using TypeMap = std::map<const char*, instance::TypeView>;
    struct Types {
        TypeMap map{};
        Parameters parameters{};
        // TODO(arBmind): add instance types etc.
    };

    Types& types;
    instance::Module instanceModule;

    Adapter(Types* types)
        : types(*types) {}

    void resolveTypes() {
        for (auto [parameter, typeName] : types.parameters) {
            auto typeIt = types.map.find(typeName);
            if (typeIt == types.map.end()) {
                // TODO(arBmind): error, unknown type
                continue;
            }
            parameter->typed.type.visit(
                [&](parser::Pointer& p) {
                    if (p.target) {
                        p.target->get<parser::Pointer>().target =
                            std::make_shared<parser::TypeExpression>(parser::TypeInstance{typeIt->second});
                    }
                    else {
                        p.target = std::make_shared<parser::TypeExpression>(parser::TypeInstance{typeIt->second});
                    }
                },
                [&, a = parameter](auto) { //
                    a->typed.type = parser::TypeInstance{typeIt->second};
                });
        }
        // TODO(arBmind): resolve other types
    }

    template<class T, class R>
    void constructedType(R (*construct)()) {
        // TODO(arBmind): normal type + construct & destruct functions
        (void)construct;
    }

    template<class T, class R, class Param>
    void constructedType(R (*construct)(const Param&)) {
        (void)construct;
        using namespace intrinsic;
        constexpr auto info = TypeOf<T>::info();

        auto r = instance::Function{};
        r.name = strings::to_string(info.name);
        // r.flags =;
        // r.parameters = typeParameters(&TypeOf<T>::eval);
        // r.body =;
        // call T::eval(…)
        // return Type that stores result & all functions

        instanceModule.locals.emplace(std::move(r));
    }

    static constexpr auto typeFlags(intrinsic::TypeFlags flags) -> instance::TypeFlags {
        auto r = instance::TypeFlags{};
        (void)flags;
        // TODO(arBmind)
        return r;
    }

    static constexpr auto typeParser(intrinsic::Parser parser) -> instance::Parser {
        using namespace intrinsic;
        switch (parser) {
        case Parser::Expression: return instance::Parser::Expression;
        case Parser::SingleToken: return instance::Parser::SingleToken;
        case Parser::IdTypeValue: return instance::Parser::IdTypeValue;
        case Parser::IdTypeValueTuple: return instance::Parser::IdTypeValueTuple;
        case Parser::OptionalIdTypeValueTuple: return instance::Parser::OptionalIdTypeValueTuple;
        }
        return {};
    }

    template<class R, class... Params>
    auto typeParameters(R (*)(Params...)) -> instance::Parameters {
        return {parameter<Params>()..., typeResultParameter()};
    }

    auto typeResultParameter() -> instance::Parameter {
        auto r = instance::Parameter{};
        r.typed.name = strings::String{"result"};
        // r.typed.type = // prosponed "instance::Type"
        r.side = instance::ParameterSide::result;
        // r.flags |= instance::ParameterFlag::assignable; // TODO(arBmind): missing
        return r;
    }

    template<class... Params, size_t... I>
    auto trackParameters(instance::ParameterViews& args, std::index_sequence<I...>) {
        using namespace intrinsic;
        (types.parameters.push_back(
             ParameterRef{const_cast<instance::Parameter*>(args[I]), Parameter<Params>::typeInfo().name.data()}),
         ...);
    }

    template<class T>
    auto parameter(instance::LocalScope& scope) -> instance::ParameterView {
        using namespace intrinsic;

        auto node = scope.emplace([] {
            constexpr auto info = Parameter<T>::info();
            auto r = instance::Parameter{};
            r.typed.name = strings::to_string(info.name);
            if (info.flags.any(ParameterFlag::Assignable, ParameterFlag::Reference)) {
                if (Parameter<T>::is_pointer) {
                    r.typed.type = parser::Pointer{std::make_shared<parser::TypeExpression>(parser::Pointer{})};
                }
                else
                    r.typed.type = parser::Pointer{};
            }
            else if (Parameter<T>::is_pointer) {
                r.typed.type = parser::Pointer{};
            }
            // r.typed.type = // this has to be delayed until all types are known
            r.side = parameterSide(info.side);
            r.flags = parameterFlags(info.flags);
            return r;
        }());
        return &node->template get<instance::Parameter>();
    }

    constexpr static auto functionFlags(intrinsic::FunctionFlags flags) -> instance::FunctionFlags {
        using namespace intrinsic;
        auto r = instance::FunctionFlags{};
        if (flags.any(FunctionFlag::CompileTimeOnly)) {
            r |= instance::FunctionFlag::compiletime;
        }
        if (flags.any(FunctionFlag::CompileTimeSideEffects)) {
            r |= instance::FunctionFlag::compiletime;
            r |= instance::FunctionFlag::compiletime_sideeffects;
        }
        return r;
    }

    constexpr static auto parameterSide(intrinsic::ParameterSide side) -> instance::ParameterSide {
        using namespace intrinsic;
        switch (side) {
        case ParameterSide::Left: return instance::ParameterSide::left;
        case ParameterSide::Right: return instance::ParameterSide::right;
        case ParameterSide::Result: return instance::ParameterSide::result;
        case ParameterSide::Implicit: assert(false); return {}; // should be filtered before
        }
        return {};
    }

    constexpr static auto parameterFlags(intrinsic::ParameterFlags flags) -> instance::ParameterFlags {
        using namespace intrinsic;
        auto r = instance::ParameterFlags{};
        if (flags.any(ParameterFlag::Assignable)) {
            r |= instance::ParameterFlag::assignable;
        }
        if (flags.any(ParameterFlag::Unrolled)) {
            r |= instance::ParameterFlag::splatted;
        }
        return r;
    }
};

} // namespace intrinsicAdapter
