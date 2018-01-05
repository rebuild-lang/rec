#include "intrinsic/Function.h"

#include "instance/Type.h"
#include "strings/String.h"

#include "gtest/gtest.h"
#include <array>
#include <iostream>
#include <vector>

struct List {
    using Type = instance::Type;
    using Value = int; // TODO
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
    static constexpr auto info = [] {
        auto info = TypeInfo{};
        info.name = "u64";
        info.size = sizeof(uint64_t);
        info.flags = TypeFlag::CompileTime | TypeFlag::RunTime;
        return info;
    }();

    struct Result {
        using type = uint64_t;
        static constexpr auto info = [] {
            auto info = ArgumentInfo{};
            info.name = "result";
            info.side = ArgumentSide::Result;
            info.flags = ArgumentFlag::Assignable;
            return info;
        }();
    };

    struct ImplicitFromLiteral {
        static constexpr auto info = [] {
            auto info = FunctionInfo{};
            info.name = ".implicitFrom";
            info.flags = FunctionFlag::CompileTimeOnly;
            return info;
        }();
        struct Literal {
            using type = uint64_t; // TODO: NumberLiteral
            static constexpr auto info = [] {
                auto info = ArgumentInfo{};
                info.name = "literal";
                info.side = ArgumentSide::Right;
                return info;
            }();
        };

        static void eval(const Arg<Literal>& literal, Arg<Result>& res) {
            // TODO
        }
    };

    struct Add {
        static constexpr auto info = [] {
            auto info = FunctionInfo{};
            info.name = "add";
            info.flags = {};
            return info;
        }();

        struct Left {
            using type = uint64_t;
            static constexpr auto info = [] {
                auto info = ArgumentInfo{};
                info.name = "left";
                info.side = ArgumentSide::Left;
                return info;
            }();
        };
        struct Right {
            using type = uint64_t;
            static constexpr auto info = [] {
                auto info = ArgumentInfo{};
                info.name = "right";
                info.side = ArgumentSide::Right;
                return info;
            }();
        };

        static void eval(Arg<Left> l, Arg<Right> r, Arg<Result>& res) { //
            res.v = l.v + r.v;
        }
    };

    static void _register(RegisterModule& mod) {
        mod.function<ImplicitFromLiteral>();
        mod.function<Add>();
        // mod.function<&sub>();
        // mod.function<&mul>();
    }
};

template<>
struct TypeOf<String> {
    static constexpr auto info = [] {
        auto info = TypeInfo{};
        info.name = "str";
        info.size = sizeof(String);
        info.flags = TypeFlag::CompileTime | TypeFlag::Construct;
        return info;
    }();

    static void _register(RegisterModule& mod) {
        // mod.function<ImplicitFromLiteral>();
        // mod.function<Length>();
        // mod.function<At>();
        // mod.function<Append>();
    }
};

// TODO
// template<> struct TypeOf<Rope> {};

template<>
struct TypeOf<Flags> {
    static constexpr auto info = [] {
        auto info = TypeInfo{};
        info.name = "Flags";
        info.size = sizeof(uint64_t);
        info.flags = TypeFlag::CompileTime | TypeFlag::Construct;
        return info;
    }();

    struct IdentifierLiterals {
        using type = std::vector<uint64_t>; // TODO: parser::Identifier
        static constexpr auto info = [] {
            auto info = ArgumentInfo{};
            info.name = "ids";
            info.side = ArgumentSide::Right;
            info.flags = ArgumentFlag::Unrolled;
            return info;
        }();
    };

    using TypeData = std::vector<uint64_t>; // TODO: parser::Identifier

    static auto eval(const Arg<IdentifierLiterals>& ids) -> TypeData { return {ids.v}; }

    static auto construct(const TypeData& typeData) -> Flags { return {typeData}; }
    static auto destruct(Flags& flags) { flags.~Flags(); }
};

template<>
struct TypeOf<List> {
    static constexpr auto info = [] {
        auto info = TypeInfo{};
        info.name = "List";
        info.size = sizeof(List);
        info.flags = TypeFlag::CompileTime | TypeFlag::Construct;
        return info;
    }();

    struct Result {
        using type = List;
        static constexpr auto info = [] {
            auto info = ArgumentInfo{};
            info.name = "result";
            info.side = ArgumentSide::Result;
            info.flags = ArgumentFlag::Assignable;
            return info;
        }();
    };

    struct TypeArgument {
        using type = instance::Type*;
        static constexpr auto info = [] {
            auto info = ArgumentInfo{};
            info.name = "type";
            info.side = ArgumentSide::Right;
            return info;
        }();
    };

    struct Construct {
        static constexpr auto info = [] {
            auto info = FunctionInfo{};
            info.name = ".construct";
            info.flags = FunctionFlag::CompileTimeOnly;
            return info;
        }();

        static void eval(Arg<TypeArgument> type, Arg<Result>& res) {
            // TODO
        }
    };

    using TypeData = instance::Type*;

    static auto eval(Arg<TypeArgument> type) -> TypeData { return {type.v}; }

    static auto construct(TypeData typeData) -> List { return {*typeData}; }
    static auto destruct(List& list) { list.~List(); }

    static void _register(RegisterModule& mod) {
        // mod.function<Construct>();
        // mod.function<Destruct>();
        // mod.function<Length>();
        // mod.function<At>();
    }
};

template<>
struct TypeOf<instance::TypeFlags> {
    static constexpr auto info = [] {
        auto info = TypeInfo{};
        info.name = "TypeFlags";
        info.size = sizeof(instance::TypeFlags);
        info.flags = intrinsic::TypeFlag::CompileTime | intrinsic::TypeFlag::Instance;
        return info;
    }();

    using evalType = Flags;
    static auto constructArguments() -> TypeOf<Flags>::TypeData {
        // constexpr auto names = std::array<const char*, 2>{{"CompileTime", "RunTime"}};
        return {}; // TODO
    }
};

template<>
struct TypeOf<instance::Type> {
    static constexpr auto info = [] {
        auto info = TypeInfo{};
        info.name = "Type";
        info.size = sizeof(instance::Type);
        info.flags = TypeFlag::CompileTime;
        return info;
    }();

    struct ThisArgument {
        using type = instance::Type;
        static constexpr auto info = [] {
            auto info = ArgumentInfo{};
            info.name = "this";
            info.side = ArgumentSide::Left;
            return info;
        }();
    };

    struct ReadFlags {
        static constexpr auto info = [] {
            auto info = FunctionInfo{};
            info.name = ".flags";
            info.flags = FunctionFlag::CompileTimeOnly;
            return info;
        }();

        struct Result {
            using type = instance::TypeFlags;
            static constexpr auto info = [] {
                auto info = ArgumentInfo{};
                info.name = "result";
                info.side = ArgumentSide::Result;
                info.flags = ArgumentFlag::Assignable;
                return info;
            }();
        };

        static void eval(const Arg<ThisArgument>& self, Arg<Result>& res) {
            // TODO
        }
    };

    static void _register(RegisterModule& mod) {
        // mod.function<ReadName>();
        // mod.function<ReadParent>();
        // mod.function<ReadFlags>();
        // mod.function<ReadSize>();
        // mod.function<Construct>();
        // mod.function<Destruct>();
        // mod.function<ImplicitFrom>();
        // mod.function<Instances>(); // ???
    }
};

} // namespace intrinsic

TEST(intrinsic, output) {
    using namespace intrinsic;
    _register("Rebuild", [](RegisterModule & rebuild) noexcept { //
        rebuild.type<uint64_t>();
    });
}
