#include "intrinsic/Function.h"

#include "gtest/gtest.h"

#include <iostream>

namespace intrinsic {

template<>
struct TypeOf<uint64_t> {
    static constexpr auto info() noexcept {
        auto res = TypeInfo{};
        res.name = "u64";
        res.size = 8;
        res.flags = TypeFlag::CompileTime | TypeFlag::RunTime;
        return res;
    }
};

struct AddFunc {
    static constexpr auto info() noexcept {
        auto res = FunctionInfo{};
        res.name = ".Add";
        res.flags = {};
        return res;
    }

    struct Left {
        using type = uint64_t;
        static constexpr auto info() noexcept -> ArgumentInfo {
            auto res = ArgumentInfo{};
            res.name = "left";
            res.side = ArgumentSide::Left;
            return res;
        }
    };
    struct Right {
        using type = uint64_t;
        static constexpr auto info() noexcept -> ArgumentInfo {
            auto res = ArgumentInfo{};
            res.name = "right";
            res.side = ArgumentSide::Right;
            return res;
        }
    };
    struct Result {
        using type = uint64_t;
        static constexpr auto info() noexcept -> ArgumentInfo {
            auto res = ArgumentInfo{};
            res.name = "result";
            res.side = ArgumentSide::Result;
            res.flags = ArgumentFlag::Assignable;
            return res;
        }
    };

    static void eval(Arg<Left> l, Arg<Right> r, Arg<Result> &res) { //
        res.v = l.v + r.v;
    }
};

} // namespace intrinsic

TEST(intrinsic, output) {
    using namespace intrinsic;
    _register("Rebuild", [](RegisterModule & rebuild) noexcept {
        rebuild.type<uint64_t>([](RegisterModule & u64) noexcept {
            u64.function<AddFunc>();
            // u64.function<&sub>();
            // u64.function<&mul>();
        });
    });
}
