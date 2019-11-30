#include "Variant.ostream.h"

#include "gtest/gtest.h"

namespace meta {

constexpr auto nameOf(Type<void>) { return "void"; }
constexpr auto nameOf(Type<int>) { return "int"; }
constexpr auto nameOf(Type<double>) { return "double"; }

} // namespace meta

TEST(variant, normal) {
    using TestVariant = meta::Variant<int, float>;

    auto v = TestVariant{23};
    EXPECT_FALSE(v.holds<float>());
    ASSERT_TRUE(v.holds<int>());
    EXPECT_EQ(v.get<int>(), 23);
    EXPECT_EQ(v, TestVariant{23});

    EXPECT_EQ(v.index(), TestVariant::indexOf<int>());

    EXPECT_TRUE(v.visit([](int) { return true; }, [](auto) { return false; }));

    {
        auto visited = bool{};
        v.visitSome([&](double) { visited = true; }); // compiles even though it could not macht!!
        ASSERT_FALSE(visited);
    }
    {
        auto visited = bool{};
        v.visitSome([&](int) { visited = true; });
        ASSERT_TRUE(visited);
    }

    // ASSERT_EQ(TestVariant{23.}, TestVariant{23}); // Trigger Failure Output
}

using DerivedVariant = meta::Variant<int, float>;
struct Derived final : DerivedVariant {
    int member{};

    META_VARIANT_CONSTRUCT(Derived, DerivedVariant)
};
TEST(variant, derived) {
    auto v = Derived{23};
    v.member = 42;

    EXPECT_FALSE(v.holds<float>());
    ASSERT_TRUE(v.holds<int>());
    EXPECT_EQ(v.get<int>(), 23);
    EXPECT_EQ(v, Derived{23});

    EXPECT_EQ(v.index(), Derived::indexOf<int>());

    EXPECT_TRUE(v.visit([](int) { return true; }, [](auto) { return false; }));

    auto b = v;
    EXPECT_EQ(b.member, v.member);
    EXPECT_EQ(b, Derived{23});
}

TEST(variant, ostream_simple) {
    using TestVariant = meta::Variant<int, float>;

    auto ss = std::stringstream{};
    ss << TestVariant{3.14f};
    ASSERT_EQ(ss.str(), "3.14");
}

TEST(variant, ostream_annotated) {
    using TestVariant = meta::Variant<int, double>;

    auto ss = std::stringstream{};
    ss << TestVariant{3.14};
    ASSERT_EQ(ss.str(), "[double]3.14");
}

TEST(variant, ostream_index) {
    using TestVariantIndex = meta::VariantIndex<int, double>;
    static_assert(meta::details::all_named<meta::TypePack<int, double>>);

    auto ss = std::stringstream{};
    ss << TestVariantIndex{1};
    ASSERT_EQ(ss.str(), "double");
}
