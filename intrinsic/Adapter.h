#pragma once
#include "intrinsic/Argument.h"
#include "intrinsic/Context.h"
#include "intrinsic/Function.h"
#include "intrinsic/Module.h"
#include "intrinsic/Type.h"

#include "instance/Argument.h"
#include "instance/Function.h"
#include "instance/Module.h"
#include "instance/Node.h"
#include "instance/Scope.h"
#include "instance/Type.h"

#include "tools/meta/TypeList.h"

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

template<class Arg>
struct ArgumentAt {
    static auto from(uint8_t* memory) -> Arg { return *reinterpret_cast<Arg*>(memory); }
};

template<class Arg>
struct ArgumentAt<Arg&> {
    static auto from(uint8_t* memory) -> Arg& { return **reinterpret_cast<Arg**>(memory); }
};

template<class Arg>
struct ArgumentAt<const Arg&> {
    static auto from(uint8_t* memory) -> Arg& { return **reinterpret_cast<Arg**>(memory); }
};

template<class Arg>
constexpr auto argumentSize() -> size_t {
    using namespace intrinsic;
    if constexpr (Argument<Arg>::info().side == ArgumentSide::Implicit) {
        return {};
    }
    else if constexpr (Argument<Arg>::info().flags.any(ArgumentFlag::Assignable, ArgumentFlag::Reference)) {
        return sizeof(void*);
    }
    else {
        return Argument<Arg>::typeInfo().size;
    }
}

template<auto* F, class... Args>
struct Call {
    using Func = void (*)(Args...);
    using Sizes = std::index_sequence<argumentSize<Args>()...>;
    using Indices = std::make_index_sequence<sizeof...(Args)>;

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
        f(argumentAt<Args, Offset>(memory, context)...);
    }

    template<class Arg, size_t Offset>
    static auto argumentAt(uint8_t* memory, intrinsic::Context* context) -> Arg {
        using namespace intrinsic;
        if constexpr (Argument<Arg>::info().side == ArgumentSide::Implicit) {
            return {context};
        }
        else {
            return ArgumentAt<Arg>::from(memory + Offset);
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
            auto a = Adapter{&types};
            a.moduleName(info.name);

            TypeOf<T>::module(a);

            a.instanceModule.locals.emplace([]() -> instance::Type {
                constexpr auto info = TypeOf<T>::info();
                auto r = instance::Type{};
                r.name = instance::Name{"type"};
                r.size = info.size;
                r.flags = typeFlags(info.flags);
                r.parser = typeParser(info.parser);
                return r;
            }());

            auto optNode = a.instanceModule.locals[instance::Name{"type"}];
            assert(optNode);
            types.map[info.name.data()] = &optNode.value()->get<instance::Type>();

            instanceModule.locals.emplace(std::move(a.instanceModule));

            auto modNode = instanceModule.locals[info.name];
            assert(modNode != nullptr);
            optNode.value()->get<instance::Type>().module = &modNode.value()->get<instance::Module>();
        }
    }

    template<class T>
    void module() {
        constexpr auto info = T::info();
        auto inner = Adapter{&types};
        inner.moduleName(info.name);
        T::module(inner);

        instanceModule.locals.emplace(std::move(inner.instanceModule));
    }

    using FunctionInfoFunc = intrinsic::FunctionInfo (*)();

    template<auto* F, FunctionInfoFunc Info>
    void function() {
        return functionImpl<Info, F>(makeSignature(F));
    }

    struct IsImplicit {
        template<class Arg>
        constexpr bool operator()(meta::Type<Arg>) const {
            using namespace intrinsic;
            return Argument<Arg>::info().side == ArgumentSide::Implicit;
        }
    };

    template<FunctionInfoFunc Info, auto* F, class... Args>
    void functionImpl(intrinsic::FunctionSignature<void, Args...>) {
        auto externArgs = meta::TypeList<Args...>::filterPred<IsImplicit>();
        functionImpl2<Info, F, Args...>(externArgs);
    }

    template<FunctionInfoFunc Info, auto* F, class... Args, class... ExternArgs>
    void functionImpl2(meta::TypeList<ExternArgs...>) {
        // assert((GenericFunc)f2 == (GenericFunc)F);
        auto info = Info();
        auto r = instance::Function{};
        r.name = info.name; // strings::to_string(info.name);
        // r.flags = functionFlags(info.flags); // TODO(arBmind)
        r.arguments = instance::Arguments{argument<ExternArgs>()...};

        auto call = &details::Call<F, Args...>::call;
        r.body.block.nodes.emplace_back(parser::expression::IntrinsicCall{call});

        auto indices = std::make_index_sequence<sizeof...(ExternArgs)>{};
        trackArguments<ExternArgs...>(r.arguments, indices);

        instanceModule.locals.emplace(std::move(r));
    }

    void moduleName(intrinsic::Name name) { instanceModule.name = name; } // strings::to_string(name); }

private:
    struct ArgumentRef {
        instance::Argument* ref;
        const char* typeName;
    };
    using Arguments = std::vector<ArgumentRef>;
    using TypeMap = std::map<const char*, instance::TypeView>;
    struct Types {
        TypeMap map{};
        Arguments arguments{};
        // TODO(arBmind): add instance types etc.
    };

    Types& types;
    instance::Module instanceModule;

    Adapter(Types* types)
        : types(*types) {}

    void resolveTypes() {
        for (auto [argument, typeName] : types.arguments) {
            auto typeIt = types.map.find(typeName);
            if (typeIt == types.map.end()) {
                // TODO(arBmind): error, unknown type
                continue;
            }
            argument->typed.type.visit(
                [&](parser::expression::Pointer& p) {
                    p.target = std::make_shared<parser::expression::TypeExpression>(
                        parser::expression::TypeInstance{typeIt->second});
                },
                [&, a = argument](auto) { a->typed.type = parser::expression::TypeInstance{typeIt->second}; });
        }
        // TODO(arBmind): resolve other types
    }

    template<class T, class R>
    void constructedType(R (*construct)()) {
        // TODO(arBmind): normal type + construct & destruct functions
        (void)construct;
    }

    template<class T, class R, class Arg>
    void constructedType(R (*construct)(const Arg&)) {
        (void)construct;
        using namespace intrinsic;
        constexpr auto info = TypeOf<T>::info();

        auto r = instance::Function{};
        r.name = info.name; // strings::to_string(info.name);
        // r.flags =;
        r.arguments = typeArguments(&TypeOf<T>::eval);
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

    template<class R, class... Args>
    auto typeArguments(R (*)(Args...)) -> instance::Arguments {
        return {argument<Args>()..., typeResultArgument()};
    }

    auto typeResultArgument() -> instance::Argument {
        auto r = instance::Argument{};
        r.typed.name = strings::String{"result"};
        // r.typed.type = // prosponed "instance::Type"
        r.side = instance::ArgumentSide::result;
        // r.flags |= instance::ArgumentFlag::assignable; // TODO(arBmind): missing
        return r;
    }

    template<class... Args, size_t... I>
    auto trackArguments(instance::Arguments& args, std::index_sequence<I...>) {
        using namespace intrinsic;
        (types.arguments.push_back(ArgumentRef{&args[I], Argument<Args>::typeInfo().name.data()}), ...);
    }

    template<class T>
    auto argument() -> instance::Argument {
        using namespace intrinsic;
        constexpr auto info = Argument<T>::info();
        auto r = instance::Argument{};
        r.typed.name = info.name;
        if (info.flags.any(ArgumentFlag::Assignable, ArgumentFlag::Reference)) {
            r.typed.type = parser::expression::Pointer{};
        }
        // r.typed.type = // this has to be delayed until all types are known
        r.side = argumentSide(info.side);
        r.flags = argumentFlags(info.flags);
        return r;
    }

    constexpr static auto argumentSide(intrinsic::ArgumentSide side) -> instance::ArgumentSide {
        using namespace intrinsic;
        switch (side) {
        case ArgumentSide::Left: return instance::ArgumentSide::left;
        case ArgumentSide::Right: return instance::ArgumentSide::right;
        case ArgumentSide::Result: return instance::ArgumentSide::result;
        case ArgumentSide::Implicit: assert(false); return {}; // should be filtered before
        }
        return {};
    }

    constexpr static auto argumentFlags(intrinsic::ArgumentFlags flags) -> instance::ArgumentFlags {
        using namespace intrinsic;
        auto r = instance::ArgumentFlags{};
        if (flags.any(ArgumentFlag::Assignable)) {
            r |= instance::ArgumentFlag::assignable;
        }
        if (flags.any(ArgumentFlag::Unrolled)) {
            r |= instance::ArgumentFlag::splatted;
        }
        return r;
    }
}; // namespace intrinsicAdapter

} // namespace intrinsicAdapter
