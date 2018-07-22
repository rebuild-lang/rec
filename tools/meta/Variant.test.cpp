#include "Variant.ostream.h"

#include "gtest/gtest.h"

TEST(variant, normal) {
    using TestVariant = meta::Variant<int, float>;

    auto v = TestVariant{23};
    ASSERT_TRUE(v.holds<int>());
    ASSERT_FALSE(v.holds<float>());
    ASSERT_EQ(v.get<int>(), 23);
    ASSERT_EQ(v, TestVariant{23});

    ASSERT_EQ(v.index(), TestVariant::indexOf<int>());

    ASSERT_TRUE(v.visit([](int) { return true; }, [](auto) { return false; }));

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

TEST(variant, ostream_simple) {
    using TestVariant = meta::Variant<int, float>;

    auto ss = std::stringstream{};
    ss << TestVariant{3.14f};
    ASSERT_EQ(ss.str(), "3.14");
}

constexpr auto nameOf(meta::Type<int>) { return "int"; }
constexpr auto nameOf(meta::Type<double>) { return "double"; }

TEST(variant, ostream_annotated) {
    using TestVariant = meta::Variant<int, double>;

    auto ss = std::stringstream{};
    ss << TestVariant{3.14};
    ASSERT_EQ(ss.str(), "[double]3.14");
}

TEST(variant, ostream_index) {
    using TestVariantIndex = meta::VariantIndex<int, double>;

    auto ss = std::stringstream{};
    ss << TestVariantIndex{1};
    ASSERT_EQ(ss.str(), "double");
}
