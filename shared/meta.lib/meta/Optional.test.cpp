#include "Optional.ostream.h"

#include "gtest/gtest.h"

TEST(optional, normal) {
    using OptInt = meta::Optional<int>;

    static_assert(sizeof(OptInt) > sizeof(int));

    auto optInt = OptInt{};
    ASSERT_FALSE(optInt);
    ASSERT_EQ(optInt, OptInt{});

    ASSERT_EQ(optInt.orValue(42), 42);
    ASSERT_EQ(std::move(optInt).orValue(42), 42);

    const auto& constOpt = optInt;
    ASSERT_FALSE(constOpt);
    ASSERT_EQ(constOpt.orValue(42), 42);

    ASSERT_FALSE(constOpt && [](int v) { return v > 42; });

    optInt = 23;
    ASSERT_TRUE(optInt);
    ASSERT_EQ(constOpt, 23);

    ASSERT_EQ(optInt.value(), 23);
    ASSERT_EQ(optInt.orValue(42), 23);
    ASSERT_EQ(std::move(optInt).orValue(42), 23);

    auto l = constOpt.map([](int v) { return v * 2; });
    ASSERT_EQ(l, 23 * 2);

    ASSERT_FALSE(constOpt && [](int v) { return v > 42; });
    ASSERT_TRUE(constOpt && [](int v) { return v > 0; });
}

TEST(optional, invalidFunc) {
    struct IntInvalid {
        constexpr int operator()() { return -1; }
    };
    using OptInt = meta::Optional<meta::Packed<IntInvalid>>;

    static_assert(sizeof(OptInt) == sizeof(int));
    {
        constexpr auto optInt = OptInt{};
        static_assert(!optInt);
        static_assert(optInt == OptInt{});
        static_assert(optInt.value() == IntInvalid{}());

        static_assert(optInt.orValue(42) == 42);
        static_assert(std::move(optInt).orValue(42) == 42);

        constexpr const auto& constOpt = optInt;
        static_assert(!constOpt);
        static_assert(constOpt.orValue(42) == 42);

        static_assert(!(constOpt && [](int v) { return v > 42; }));
    }
    {
        constexpr auto optInt = OptInt{23};
        static_assert(optInt);
        static_assert(optInt == 23);

        static_assert(optInt.value() == 23);
        static_assert(optInt.orValue(42) == 23);
        static_assert(std::move(optInt).orValue(42) == 23);

        constexpr const auto& constOpt = optInt;
        constexpr auto l = constOpt.map([](int v) { return v * 2; });
        static_assert(l == 23 * 2);

        static_assert(!(constOpt && [](int v) { return v > 42; }));
        static_assert(constOpt && [](int v) { return v > 0; });
    }

    auto optInt = OptInt{};
    ASSERT_NE(optInt, 23);
    optInt = 23;
    ASSERT_EQ(optInt, 23);
}

TEST(optional, defaultPacked) {
    using OptInt = meta::Optional<meta::DefaultPacked<int>>;

    static_assert(sizeof(OptInt) == sizeof(int));
}

TEST(optional, pointer) {
    using OptPtr = meta::Optional<int*>;

    static_assert(sizeof(OptPtr) == sizeof(int*));
}

TEST(optional, reference) {
    using OptRef = meta::Optional<int&>;

    static_assert(sizeof(OptRef) == sizeof(int*));

    {
        constexpr auto optRef = OptRef{};
        static_assert(!optRef);
        static_assert(optRef == OptRef{});
        // static_assert(&optRef.value() == nullptr);

        static_assert(optRef.orValue(42) == 42);
        static_assert(std::move(optRef).orValue(42) == 42);

        constexpr const auto& constOpt = optRef;
        static_assert(!constOpt);
        static_assert(constOpt.orValue(42) == 42);

        static_assert(!(constOpt && [](int v) { return v > 42; }));
    }
    {
        constexpr int i = 23;
        constexpr auto optRef = OptRef{const_cast<int&>(i)};
        static_assert(optRef);
        static_assert(optRef == OptRef{const_cast<int&>(i)});

        // static_assert(optRef.value() == 23);
        // static_assert(&optRef.value() == &i);
        // static_assert(optRef.orValue(42) == 23);
        // static_assert(std::move(optRef).orValue(42) == 23);

        constexpr const auto& constOpt = optRef;

        constexpr auto l = constOpt.map([](int v) { return v * 2; });
        static_assert(l == 23 * 2);

        static_assert(!(constOpt && [](int v) { return v > 42; }));
        static_assert(constOpt && [](int v) { return v > 0; });
    }

    {
        auto i = 23;
        auto optRef = OptRef{};
        ASSERT_NE(optRef, OptRef{i});
        ASSERT_EQ(&optRef.value(), nullptr);

        optRef = std::ref(i);
        ASSERT_EQ(optRef, OptRef{i});
    }
    {
        int i = 23;
        auto optRef = OptRef{std::ref(i)};
        ASSERT_EQ(optRef, OptRef{i});

        ASSERT_EQ(optRef.value(), 23);
        ASSERT_EQ(&optRef.value(), &i);

        ASSERT_EQ(optRef.orValue(42), 23);
        ASSERT_EQ(std::move(optRef).orValue(42), 23);

        const auto& constOpt = optRef;

        auto l = constOpt.map([](int v) { return v * 2; });
        ASSERT_EQ(l, 23 * 2);

        ASSERT_FALSE(constOpt && [](int v) { return v > 42; });
        ASSERT_TRUE(constOpt && [](int v) { return v > 0; });
    }
}

TEST(optional, ostream_empty) {
    using OptInt = meta::Optional<int>;
    auto ss = std::stringstream{};
    ss << OptInt{};
    ASSERT_EQ(ss.str(), "<Empty>");
}

TEST(optional, ostream_value) {
    using OptInt = meta::Optional<int>;
    auto ss = std::stringstream{};
    ss << OptInt{23};
    ASSERT_EQ(ss.str(), "23");
}
