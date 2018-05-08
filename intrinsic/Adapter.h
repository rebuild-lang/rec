#pragma once
#include "intrinsic/Argument.h"
#include "intrinsic/Function.h"
#include "intrinsic/Module.h"
#include "intrinsic/Type.h"

#include "instance/Argument.h"
#include "instance/Function.h"
#include "instance/Module.h"
#include "instance/Node.h"
#include "instance/Type.h"

#include <cassert>

namespace intrinsicAdapter {

namespace details {

template<size_t U, size_t... V, size_t... I>
constexpr auto sumN(std::index_sequence<V...>, std::index_sequence<I...>) -> size_t {
    return ((I < U ? V : 0) + ... + 0);
}

template<size_t... V, size_t... I>
constexpr auto partialSum(std::index_sequence<V...> values, std::index_sequence<I...> indices) {
    // 2018-01-31 arB note: VS2017 15.5.5 refuses to work properly with return type here!
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

template<auto* F, class... Args>
struct Call {
    using Func = void (*)(Args...);
    using Sizes = std::index_sequence<intrinsic::Argument<Args>::typeInfo().size...>;
    using Indices = std::make_index_sequence<sizeof...(Args)>;

    static void call(uint8_t* memory) {
        constexpr auto sizes = Sizes{};
        constexpr auto indices = Indices{};
        constexpr auto offsets = partialSum(sizes, indices);
        callImpl(memory, offsets);
    }

private:
    template<size_t... Offset>
    static void callImpl(uint8_t* memory, std::index_sequence<Offset...>) {
        auto f = reinterpret_cast<Func>(F);
        f(argumentAt<Args, Offset>(memory)...);
    }

    template<class Arg, size_t Offset>
    static auto argumentAt(uint8_t* memory) -> Arg {
        return ArgumentAt<Arg>::from(memory + Offset);
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
            // TODO
        }
        else if constexpr (info.flags.any(TypeFlag::Construct)) {
            constructedType<T>(&TypeOf<T>::construct);
        }
        else {
            auto a = Adapter{&types};
            a.moduleName(info.name);

            TypeOf<T>::module(a);

            auto t = instance::Type{};
            t.name = instance::Name{"type"};
            t.size = info.size;
            // r.flags = typeFlags(info.flags); // TODO
            a.instanceModule.locals.emplace(std::move(t));

            auto node = a.instanceModule.locals[instance::Name{"type"}];
            assert(node != nullptr);
            types.map[info.name.data()] = &node->get<instance::Type>();

            instanceModule.locals.emplace(std::move(a.instanceModule));
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

    template<FunctionInfoFunc Info, auto* F, class... Args>
    void functionImpl(intrinsic::FunctionSignature<void, Args...>) {
        // assert((GenericFunc)f2 == (GenericFunc)F);
        auto info = Info();
        auto r = instance::Function{};
        r.name = info.name; // strings::to_string(info.name);
        // r.flags = functionFlags(info.flags); // TODO
        r.arguments = instance::Arguments{argument<Args>()...};

        auto call = &details::Call<F, Args...>::call;
        r.body.block.nodes.emplace_back(parser::expression::IntrinsicCall{call});

        auto indices = std::make_index_sequence<sizeof...(Args)>{};
        trackArguments<Args...>(r.arguments, indices);

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
        // TODO: add instance types etc.
    };

    Types& types;
    instance::Module instanceModule;

    Adapter(Types* types)
        : types(*types) {}

    void resolveTypes() {
        for (auto [argument, typeName] : types.arguments) {
            auto typeIt = types.map.find(typeName);
            if (typeIt == types.map.end()) {
                // TODO: error, unknown type
                continue;
            }
            argument->typed.type.visit(
                [&](instance::type::Pointer& p) {
                    p.target = std::make_shared<instance::type::Expression>(instance::type::Instance{typeIt->second});
                },
                [&, a = argument](auto) { a->typed.type = instance::type::Instance{typeIt->second}; });
        }
        // TODO: resolve other types
    }

    template<class T, class R>
    void constructedType(R (*construct)()) {
        // TODO: normal type + construct & destruct functions
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

    template<class R, class... Args>
    auto typeArguments(R (*)(Args...)) -> instance::Arguments {
        return {argument<Args>()..., typeResultArgument()};
    }

    auto typeResultArgument() -> instance::Argument {
        auto r = instance::Argument{};
        r.typed.name = strings::String{"result"};
        // r.typed.type = // prosponed "instance::Type"
        r.side = instance::ArgumentSide::result;
        // r.flags |= instance::ArgumentFlag::assignable; // TODO: missing
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
            r.typed.type = instance::type::Pointer{};
        }
        // r.typed.type = // this has to be delayed until all types are known
        r.side = argumentSide(info.side);
        r.flags = argumentFlags(info.flags);
        return r;
    }

    constexpr auto argumentSide(intrinsic::ArgumentSide side) -> instance::ArgumentSide {
        using namespace intrinsic;
        switch (side) {
        case ArgumentSide::Left: return instance::ArgumentSide::left;
        case ArgumentSide::Right: return instance::ArgumentSide::right;
        case ArgumentSide::Result: return instance::ArgumentSide::result;
        }
        return {};
    }

    constexpr auto argumentFlags(intrinsic::ArgumentFlags flags) -> instance::ArgumentFlags {
        using namespace intrinsic;
        auto r = instance::ArgumentFlags{};
        if (flags.any(ArgumentFlag::Assignable)) {
            // r |= instance::ArgumentFlag:: // TODO: missing
        }
        if (flags.any(ArgumentFlag::Unrolled)) {
            r |= instance::ArgumentFlag::splatted;
        }
        return r;
    }
};

} // namespace intrinsicAdapter
