#include "CodePoint.ostream.h"

#include <gtest/gtest.h>

TEST(codepoint, decimal) {
    constexpr auto d = strings::Decimal{};

    static_assert(d.v == 0xFF);
    static_assert(d == strings::Decimal{0xFF});

    constexpr auto od = strings::OptionalDecimal{strings::Decimal{9}};
    static_assert(sizeof(od) == sizeof(d));

    static_assert(od);
    static_assert(od.value() == strings::Decimal{9});

    // EXPECT_EQ(strings::Decimal{9}, strings::Decimal{7}); // Trigger failing assert output
}

TEST(codepoint, default) {
    constexpr auto cp = strings::CodePoint{'a'};

    static_assert(cp == strings::CodePoint{'a'});
    static_assert(cp < strings::CodePoint{'b'});
    static_assert(cp < 'b');
    static_assert(cp.isLetter());

    static_assert(strings::CodePoint{'8'}.isDecimalNumber());
    static_assert(strings::CodePoint{'6'}.decimalNumber() == strings::Decimal{6});

    static_assert(cp.utf8_byteCount() == strings::Counter{1});
    struct Tmp {
        size_t pc{};
        uint8_t v{};

        void push_back(uint8_t c) {
            pc++;
            v = c;
        }
    } tmp;

    cp.utf8_encode(tmp);
    ASSERT_EQ(tmp.pc, 1);
    ASSERT_EQ(tmp.v, 'a');

    static_assert(sizeof(strings::OptionalCodePoint) == sizeof(strings::CodePoint));
}

TEST(codepoint, ostream) {
    auto cp = strings::CodePoint{0x2713};

    auto ss = std::stringstream{};
    ss << cp;
    ASSERT_EQ(ss.str(), "0x2713");
}
