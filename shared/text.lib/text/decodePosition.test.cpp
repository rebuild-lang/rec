#include "decodePosition.h"

#include "DecodedPosition.ostream.h"

#include <gtest/gtest.h>

TEST(decodePosition, simple) {
    using CP = strings::CodePoint;
    using DP = text::DecodedPosition;
    using CPP = text::CodePointPosition;
    using NP = text::NewlinePosition;
    using DEP = text::DecodedErrorPosition;
    using P = text::Position;
    using Col = text::Column;
    using Line = text::Line;

    auto source = strings::View{"\r\n \ta\xF1"};
    auto gen = [](strings::View src) -> meta::CoEnumerator<strings::Decoded> {
        co_yield strings::DecodedCodePoint{src.skipBytes<0>().firstBytes<1>(), CP{'\r'}};
        co_yield strings::DecodedCodePoint{src.skipBytes<1>().firstBytes<1>(), CP{'\n'}};
        co_yield strings::DecodedCodePoint{src.skipBytes<2>().firstBytes<1>(), CP{' '}};
        co_yield strings::DecodedCodePoint{src.skipBytes<3>().firstBytes<1>(), CP{'\t'}};
        co_yield strings::DecodedCodePoint{src.skipBytes<4>().firstBytes<1>(), CP{'a'}};
        co_yield strings::DecodedError{src.skipBytes<5>().firstBytes<1>()};
    }(source);
    auto config = text::Config{Col{4}};

    auto e = text::decodePosition(std::move(gen), config);

    ASSERT_TRUE(e);
    ASSERT_TRUE(++e);
    EXPECT_EQ(*e, (DP{NP{source.firstBytes<2>(), P{Line{1}, Col{1}}}}));

    ASSERT_TRUE(++e);
    EXPECT_EQ(*e, (DP{CPP{source.skipBytes<2>().firstBytes<1>(), P{Line{2}, Col{1}}, CP{' '}, P{Line{2}, Col{2}}}}));

    ASSERT_TRUE(++e);
    EXPECT_EQ(*e, (DP{CPP{source.skipBytes<3>().firstBytes<1>(), P{Line{2}, Col{2}}, CP{'\t'}, P{Line{2}, Col{5}}}}));

    ASSERT_TRUE(++e);
    EXPECT_EQ(*e, (DP{CPP{source.skipBytes<4>().firstBytes<1>(), P{Line{2}, Col{5}}, CP{'a'}, P{Line{2}, Col{6}}}}));

    ASSERT_TRUE(++e);
    EXPECT_EQ(*e, (DP{DEP{source.skipBytes<5>().firstBytes<1>(), P{Line{2}, Col{6}}}}));

    ASSERT_FALSE(++e);
}
