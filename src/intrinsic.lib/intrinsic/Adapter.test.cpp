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
    uint64_t v{};
};

using String = strings::String;

namespace intrinsic {

template<>
struct TypeOf<uint64_t> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{"u64"};
        info.size = sizeof(uint64_t);
        info.flags = TypeFlag::CompileTime | TypeFlag::RunTime;
        return info;
    }

    struct Result {
        uint64_t v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"result"};
            info.side = ArgumentSide::Result;
            info.flags = ArgumentFlag::Assignable;
            return info;
        }
    };
    struct Literal {
        scanner::NumberLiteral v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"literal"};
            info.side = ArgumentSide::Right;
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
            auto info = ArgumentInfo{};
            info.name = Name{"left"};
            info.side = ArgumentSide::Left;
            return info;
        }
    };
    struct Right {
        uint64_t v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"right"};
            info.side = ArgumentSide::Right;
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
        mod.template function<&implicitFrom, [] {
            auto info = FunctionInfo{};
            info.name = Name{".implicitFrom"};
            info.flags = FunctionFlag::CompileTimeOnly;
            return info;
        }>();
        mod.template function<&add, [] {
            auto info = FunctionInfo{};
            info.name = Name{"add"};
            return info;
        }>();
        mod.template function<&sub, [] {
            auto info = FunctionInfo{};
            info.name = Name{"sub"};
            return info;
        }>();
        mod.template function<&mul, [] {
            auto info = FunctionInfo{};
            info.name = Name{"mul"};
            return info;
        }>();
    }
};

template<>
struct TypeOf<String> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{"str"};
        info.size = sizeof(String);
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
        info.size = sizeof(uint64_t);
        info.flags = TypeFlag::CompileTime | TypeFlag::Construct;
        return info;
    }

    struct IdentifierLiterals {
        std::vector<uint64_t> v; // TODO(arBmind): parser::Identifier
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"ids"};
            info.side = ArgumentSide::Right;
            info.flags = ArgumentFlag::Unrolled;
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
        info.size = sizeof(List);
        info.flags = TypeFlag::CompileTime | TypeFlag::Construct;
        return info;
    }

    struct Result {
        List v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"result"};
            info.side = ArgumentSide::Result;
            info.flags = ArgumentFlag::Assignable;
            return info;
        }
    };

    struct TypeArgument {
        instance::Type* v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"type"};
            info.side = ArgumentSide::Right;
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
struct TypeOf<instance::TypeFlags> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{"TypeFlags"};
        info.size = sizeof(instance::TypeFlags);
        info.flags = intrinsic::TypeFlag::CompileTime | intrinsic::TypeFlag::Instance;
        return info;
    }

    using evalType = Flags;
    static auto constructArguments() -> TypeOf<Flags>::TypeData {
        // constexpr auto names = std::array<const char*, 2>{{"CompileTime", "RunTime"}};
        return {}; // TODO(arBmind)
    }
};

template<>
struct TypeOf<instance::Type*> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{"Type"};
        info.size = sizeof(instance::Type);
        info.flags = TypeFlag::CompileTime;
        return info;
    }

    struct ThisArgument {
        instance::Type* v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"this"};
            info.side = ArgumentSide::Left;
            // info.flags = ArgumentFlag::Reference;
            return info;
        }
    };
    struct Result {
        instance::TypeFlags v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"result"};
            info.side = ArgumentSide::Result;
            info.flags = ArgumentFlag::Assignable;
            return info;
        }
    };
    static void readFlags(ThisArgument self, Result& res) { //
        res.v = self.v->flags;
    }

    template<class Module>
    static constexpr void module(Module& mod) {
        // mod.function<ReadName>();
        // mod.function<ReadParent>();
        mod.template function<&readFlags, [] {
            auto info = FunctionInfo{};
            info.name = Name{".flags"};
            info.flags = FunctionFlag::CompileTimeOnly;
            return info;
        }>();
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
        info.size = sizeof(scanner::NumberLiteral);
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
    auto rebuild = Adapter::moduleInstance<Rebuild>();
    EXPECT_EQ(strings::to_string(rebuild.name), strings::String{"Rebuild"});
}

TEST(intrinsic, call) {
    using namespace intrinsic;
    using View = strings::View;
    using Adapter = intrinsicAdapter::Adapter;
    auto rebuild = Adapter::moduleInstance<Rebuild>();
    ASSERT_TRUE(rebuild.locals[View{"u64"}]);
    ASSERT_TRUE(rebuild.locals[View{"u64"}].value()->holds<instance::Module>());
    const auto& u64 = rebuild.locals[View{"u64"}].value()->get<instance::Module>();

    ASSERT_TRUE(u64.locals[View{"add"}]);
    ASSERT_TRUE(u64.locals[View{"add"}].value()->holds<instance::Function>());
    const auto& add = u64.locals[View{"add"}].value()->get<instance::Function>();

    ASSERT_TRUE(!add.body.block.nodes.empty());
    ASSERT_TRUE(add.body.block.nodes.front().holds<parser::IntrinsicCall>());
    auto& call = add.body.block.nodes.front().get<parser::IntrinsicCall>();

    constexpr auto u64_size = intrinsic::TypeOf<uint64_t>::info().size;
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
