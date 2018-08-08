
#include "api/basic/u64.h"

#include "strings/Rope.ostream.h"

#include <gtest/gtest.h>

using Radix = scanner::Radix;

struct U64ImplicitFromData {
    const char* name{};
    strings::Rope integerPart{};
    Radix radix{};

    // expected
    uint64_t expected{};
};

static auto operator<<(std::ostream& out, const Radix& r) -> std::ostream& { return out << static_cast<int>(r); }

static auto operator<<(std::ostream& out, const U64ImplicitFromData& id) -> std::ostream& {
    return out << "name: " << id.name << "; "
               << "input: " << id.integerPart << "; "
               << "radix: " << id.radix << "; "
               << "expected: " << id.expected << '\n';
}

class U64Tests : public testing::TestWithParam<U64ImplicitFromData> {};

TEST_P(U64Tests, implicitFrom) {
    const U64ImplicitFromData& data = GetParam(); //

    auto numlit = parser::NumberLiteral{};
    numlit.value.integerPart = data.integerPart;
    numlit.value.radix = data.radix;

    auto result = intrinsic::TypeOf<api::U64>::Result{};
    intrinsic::TypeOf<api::U64>::implicitFrom({numlit}, result);

    EXPECT_EQ(data.expected, result.v);
}

INSTANTIATE_TEST_CASE_P(
    simple,
    U64Tests,
    ::testing::Values(
        U64ImplicitFromData{"zero", strings::Rope{strings::View{"0"}}, Radix::decimal, 0},
        U64ImplicitFromData{"999", strings::Rope{strings::View{"999"}}, Radix::decimal, 999},
        U64ImplicitFromData{"0x999", strings::Rope{strings::View{"999"}}, Radix::hex, 0x999} //
        ));
