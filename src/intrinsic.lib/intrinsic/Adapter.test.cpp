#include "intrinsic/Function.h"
#include "intrinsic/Module.h"
#include "intrinsic/ModuleOutput.h"

#include "intrinsic/Adapter.h"

#include "instance/Scope.h"
#include "instance/Type.h"
#include "scanner/NumberLiteralValue.h"
#include "scanner/Token.ostream.h"

#include "strings/String.h"

#include "gtest/gtest.h"
#include <array>
#include <iostream>
#include <vector>

struct List {
    using Type = instance::Type;
    using Value = int; // TODO(arBmind)
    using Index = uint64_t;

    List(Type& elementType)
        : elementType(&elementType) {}

    auto length() const -> Index { return m.size() / elementType->size; }
    // auto at(Index i) const -> Value {  }

    // auto append(Value v) const -> List;
    // auto replace(Index i, Value v) const -> List;
    // auto remove(Index i) const -> List;

private:
    Type* elementType{};
    std::vector<uint8_t> m{};
};

struct Flags {
    Flags(std::vector<uint64_t> ids)
        : ids(ids) {}

private:
    std::vector<uint64_t> ids{};
    // uint64_t v{};
};

using String = strings::String;

namespace intrinsic {

template<>
struct TypeOf<uint64_t> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{"u64"};
        info.flags = TypeFlag::CompileTime | TypeFlag::RunTime;
        return info;
    }

    struct Result {
        uint64_t v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"result"};
            info.side = ParameterSide::Result;
            info.flags = ParameterFlag::Assignable;
            return info;
        }
    };
    struct Literal {
        scanner::NumberLiteral v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"literal"};
            info.side = ParameterSide::Right;
            // info.flags = ArgumentFlag::Reference;
            return info;
        }
    };
    static void implicitFrom(Literal literal, Result& res) {
        // TODO(arBmind)
        (void)literal;
        (void)res;
    }

    struct Left {
        uint64_t v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"left"};
            info.side = ParameterSide::Left;
            return info;
        }
    };
    struct Right {
        uint64_t v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"right"};
            info.side = ParameterSide::Right;
            return info;
        }
    };
    static void add(Left l, Right r, Result& res) {
        res.v = l.v + r.v; //
    }
    static void sub(Left l, Right r, Result& res) {
        res.v = l.v - r.v; //
    }
    static void mul(Left l, Right r, Result& res) {
        res.v = l.v * r.v; //
    }

    template<class Module>
    static constexpr auto module(Module& mod) {
        mod.function(ptr_to<implicitFrom>, [] {
            auto info = FunctionInfo{};
            info.name = Name{".implicitFrom"};
            info.flags = FunctionFlag::CompileTimeOnly;
            return info;
        }());
        mod.function(ptr_to<add>, [] {
            auto info = FunctionInfo{};
            info.name = Name{"add"};
            return info;
        }());
        mod.function(ptr_to<sub>, [] {
            auto info = FunctionInfo{};
            info.name = Name{"sub"};
            return info;
        }());
        mod.function(ptr_to<mul>, [] {
            auto info = FunctionInfo{};
            info.name = Name{"mul"};
            return info;
        }());
    }
};

template<>
struct TypeOf<String> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{"str"};
        info.flags = TypeFlag::CompileTime | TypeFlag::Construct;
        return info;
    }

    static auto construct() -> String { return {}; }
    static auto destruct(String& str) { str.~String(); }

    template<class Module>
    static void module(Module& mod) {
        (void)mod;
        // mod.function<ImplicitFromLiteral>();
        // mod.function<Length>();
        // mod.function<At>();
        // mod.function<Append>();
    }
};

// TODO(arBmind)
// template<> struct TypeOf<Rope> {};

template<>
struct TypeOf<Flags> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{"flags"};
        info.flags = TypeFlag::CompileTime | TypeFlag::Construct;
        return info;
    }

    struct IdentifierLiterals {
        std::vector<uint64_t> v; // TODO(arBmind): parser::Identifier
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"ids"};
            info.side = ParameterSide::Right;
            info.flags = ParameterFlag::Unrolled;
            return info;
        }
    };

    using TypeData = std::vector<uint64_t>; // TODO(arBmind): parser::Identifier

    static auto eval(const IdentifierLiterals& ids) -> TypeData { return {ids.v}; }

    static auto construct(const TypeData& typeData) -> Flags { return {typeData}; }
    static auto destruct(Flags& flags) { flags.~Flags(); }

    template<class Module>
    static void module(Module&) {
        // TODO(arBmind)
    }
};

template<>
struct TypeOf<List> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{"list"};
        info.flags = TypeFlag::CompileTime | TypeFlag::Construct;
        return info;
    }

    struct Result {
        List v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"result"};
            info.side = ParameterSide::Result;
            info.flags = ParameterFlag::Assignable;
            return info;
        }
    };

    struct TypeArgument {
        instance::Type* v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"type"};
            info.side = ParameterSide::Right;
            return info;
        }
    };

    static void construct(TypeArgument type, Result& res) {
        // TODO(arBmind)
        (void)type;
        (void)res;
    }
    static constexpr auto constructInfo() {
        auto info = FunctionInfo{};
        info.name = Name{".construct"};
        info.flags = FunctionFlag::CompileTimeOnly;
        return info;
    }

    using TypeData = instance::Type*;
    static auto eval(TypeArgument type) -> TypeData { return type.v; }

    static auto construct(TypeData typeData) -> List { return {*typeData}; }
    static auto destruct(List& list) { list.~List(); }

    template<class Module>
    static void module(Module& mod) {
        (void)mod;
        // mod.function<Construct>();
        // mod.function<Destruct>();
        // mod.function<Length>();
        // mod.function<At>();
    }
};

template<>
struct TypeOf<instance::Type*> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{"Type"};
        info.flags = TypeFlag::CompileTime;
        return info;
    }

    struct ThisArgument {
        instance::Type* v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"this"};
            info.side = ParameterSide::Left;
            // info.flags = ArgumentFlag::Reference;
            return info;
        }
    };

    template<class Module>
    static constexpr void module(Module& mod) {
        // mod.function<ReadName>();
        // mod.function<ReadParent>();
        // mod.function<ReadSize>();
        // mod.function<Construct>();
        // mod.function<Destruct>();
        // mod.function<ImplicitFrom>();
        // mod.function<Instances>(); // ???
    }
};

template<>
struct TypeOf<scanner::NumberLiteral> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{"NumberLiteral"};
        info.flags = TypeFlag::CompileTime;
        return info;
    }

    template<class Module>
    static constexpr void module(Module&) {}
};

struct Rebuild {
    static constexpr auto info() {
        auto info = ModuleInfo{};
        info.name = Name{"Rebuild"};
        return info;
    }

    template<class Module>
    static constexpr auto module(Module& mod) {
        mod.template type<uint64_t>();
        mod.template type<String>();
        mod.template type<scanner::NumberLiteral>();
        mod.template type<instance::Type*>();
    }
};

} // namespace intrinsic

TEST(intrinsic, output) {
    using namespace intrinsic;
    auto visitor = ModuleOutput{};
    visitor.module<Rebuild>();
}

TEST(intrinsic, adapter) {
    using namespace intrinsic;
    using Adapter = intrinsicAdapter::Adapter;
    auto rebuild = Adapter::moduleOf<Rebuild>();
    EXPECT_EQ(strings::to_string(rebuild->name), strings::String{"Rebuild"});
}

TEST(intrinsic, call) {
    using namespace intrinsic;
    using View = strings::View;
    using Adapter = intrinsicAdapter::Adapter;
    auto rebuild = Adapter::moduleOf<Rebuild>();
    ASSERT_TRUE(rebuild->locals.byName(View{"u64"}).single());
    ASSERT_TRUE(rebuild->locals.byName(View{"u64"}).frontValue().holds<instance::ModulePtr>());
    const auto u64 = rebuild->locals.byName(View{"u64"}).frontValue().get<instance::ModulePtr>();

    ASSERT_TRUE(u64->locals.byName(View{"add"}).single());
    ASSERT_TRUE(u64->locals.byName(View{"add"}).frontValue().holds<instance::FunctionPtr>());
    const auto& add = u64->locals.byName(View{"add"}).frontValue().get<instance::FunctionPtr>();

    ASSERT_TRUE(add->body.holds<instance::IntrinsicCall>());
    auto& call = add->body.get<instance::IntrinsicCall>();

    constexpr auto u64_size = sizeof(uint64_t);
    constexpr auto ptr_size = sizeof(void*);
    auto result = uint64_t{};
    using Memory = std::array<uint8_t, 2 * u64_size + ptr_size>;
    auto memory = Memory{};
    reinterpret_cast<uint64_t&>(memory[0]) = 23;
    reinterpret_cast<uint64_t&>(memory[u64_size]) = 42;
    reinterpret_cast<uint64_t*&>(memory[2 * u64_size]) = &result;

    call.exec(memory.data(), nullptr);

    ASSERT_EQ(result, 23u + 42);
}
