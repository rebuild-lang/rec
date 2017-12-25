#pragma once
#include "tools/meta/Flags.h"

#include <cinttypes>

// debug only
#include <iostream>

namespace intrinsic {

enum class TypeFlag : uint64_t {
    CompileTime = 1 << 0,
    RunTime = 1 << 1,
    Construct = 1 << 2,
    Instance = 1 << 3,
};
using TypeFlags = meta::Flags<TypeFlag>;
META_FLAGS_OP(TypeFlag)

struct TypeInfo {
    const char* name{};
    uint64_t size{};
    TypeFlags flags{};
};

template<class>
struct TypeOf;

enum class ArgumentFlag : uint64_t {
    Assignable = 1 << 0,
    Unrolled = 1 << 1,
};
using ArgumentFlags = meta::Flags<ArgumentFlag>;
META_FLAGS_OP(ArgumentFlag)

enum class ArgumentSide { Left, Right, Result };

struct ArgumentInfo {
    const char* name{};
    ArgumentSide side{};
    ArgumentFlags flags{};
};

template<class T>
struct Arg {
    using type = typename T::type;
    static constexpr auto info = T::info;
    type v{};
};

enum class FunctionFlag : uint64_t {
    CompileTimeOnly = 1 << 0,
};
using FunctionFlags = meta::Flags<FunctionFlag>;
META_FLAGS_OP(FunctionFlag)

struct FunctionInfo {
    const char* name{};
    FunctionFlags flags{};
};

namespace details {

template<class T>
struct Argument;

template<class T>
struct Argument<Arg<T>> {
    using type = typename T::type;
    static constexpr auto info = T::info;
    static_assert(!info.flags.hasAny(ArgumentFlag::Assignable), "non-reference argument is not assignable");
    static constexpr auto typeInfo = TypeOf<type>::info;
};
template<class T>
struct Argument<const Arg<T>&> {
    using type = typename T::type;
    static constexpr auto info = T::info;
    static_assert(!info.flags.hasAny(ArgumentFlag::Assignable), "const-reference argument is not assignable");
    static constexpr auto typeInfo = TypeOf<type>::info;
};
template<class T>
struct Argument<Arg<T>&> : T {
    using type = typename T::type;
    static constexpr auto info = T::info;
    static_assert(info.flags.hasAny(ArgumentFlag::Assignable), "reference argument has to be assignable");
    static constexpr auto typeInfo = TypeOf<type>::info;
};

} // namespace details

struct RegisterModule {
    template<class T, class F>
    void type(F&& f) {
        std::cout << "type " << TypeOf<T>::info.name << '\n';
        auto mod = RegisterModule{};
        f(mod);
    }

    template<class T>
    void type() {
        type<T>(&TypeOf<T>::_register);
    }

    template<class T>
    void argument() {
        std::cout << "arg " << details::Argument<T>::info.name //
                  << " : " << details::Argument<T>::typeInfo.name << '\n';
    }

    template<class... Args>
    void arguments(void (*)(Args...)) {
        (argument<Args>(), ...);
    }

    template<class F>
    void function() {
        std::cout << "function " << F::info.name << '\n';
        arguments(&F::eval);
    }

    template<class F>
    void module(const char*, F&& f) {
        auto mod = RegisterModule{};
        f(mod);
    }
};

template<class F>
auto _register(const char*, F&& f) {
    auto mod = RegisterModule{};
    f(mod);
}

} // namespace intrinsic
