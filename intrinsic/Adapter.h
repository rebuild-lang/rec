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

namespace intrinsicAdapter {

/*
 * intrinsic => instance adapter
 */
struct Adapter {
    using This = Adapter;

    auto takeModule() && -> instance::Module&& { return std::move(instanceModule); }

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
            auto inner = Adapter{};
            inner.moduleName(info.name);

            TypeOf<T>::module(inner);

            auto r = instance::Type{};
            r.name = instance::Name{"type"};
            r.size = info.size;
            // r.flags = typeFlags(info.flags); // TODO
            inner.instanceModule.locals.emplace(r);

            instanceModule.locals.emplace(std::move(inner.instanceModule));
        }
    }

private:
    template<class T, class R>
    void constructedType(R (*construct)()) {
        // TODO: normal type + construct & destruct functions
    }

    template<class T, class R, class Arg>
    void constructedType(R (*construct)(const Arg&)) {
        using namespace intrinsic;
        constexpr auto info = TypeOf<T>::info();

        auto r = instance::Function{};
        r.name = strings::to_string(info.name);
        // r.flags =;
        r.arguments = typeArguments(&T::eval);
        // r.body =;
        // invoke T::eval(…)
        // return Type that stores result & all functions

        instanceModule.locals.emplace(r);
    }

    template<class R, class... Args>
    auto typeArguments(R (*)(Args...)) -> instance::Arguments {
        return {argument<Args>()..., typeResultArgument()};
    }

    auto typeResultArgument() -> instance::Argument {
        auto r = instance::Argument{};
        r.name = strings::String{"result"};
        // r.type = // prosponed "instance::Type"
        r.side = instance::ArgumentSide::result;
        // r.flags |= instance::ArgumentFlag::assignable; // TODO: missing
        return r;
    }

public:
    template<class T>
    void module() {
        constexpr auto info = T::info();
        auto inner = Adapter{};
        inner.moduleName(info.name);
        T::module(inner);

        instanceModule.locals.emplace(std::move(inner).takeModule());
    }

    using FunctionInfoFunc = intrinsic::FunctionInfo (*)();

    template<FunctionInfoFunc Info, class... Args>
    void function(void (*func)(Args...)) {
        auto info = Info();
        auto r = instance::Function{};
        r.name = strings::to_string(info.name);
        // r.flags = functionFlags(info.flags); // TODO
        r.arguments = instance::Arguments{argument<Args>()...};
        // r.body = // TODO
        instanceModule.locals.emplace(r);
    }

    void moduleName(intrinsic::Name name) { instanceModule.name = strings::to_string(name); }

private:
    instance::Module instanceModule;

    template<class T>
    auto argument() -> instance::Argument {
        using namespace intrinsic;
        constexpr auto info = Argument<T>::info();
        auto r = instance::Argument{};
        r.name = strings::to_string(info.name);
        // r.type = // this has to be delayed until all types are known
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
