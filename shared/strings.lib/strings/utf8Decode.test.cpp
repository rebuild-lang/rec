#include "utf8Decode.h"

#include "Decoded.ostream.h"

#include <gtest/gtest.h>

using strings::CodePoint;
using strings::CompareView;
using strings::Decoded;
using strings::DecodedCodePoint;
using strings::DecodedError;
using strings::utf8Decode;
using strings::View;

TEST(utf8Decode, ascii) {
    auto source = View{"aA"};

    auto e = utf8Decode(source);

    ASSERT_TRUE(e);
    ASSERT_TRUE(++e);
    ASSERT_EQ(*e, (Decoded{DecodedCodePoint{source.firstBytes<1>(), CodePoint{'a'}}}));

    ASSERT_TRUE(++e);
    ASSERT_EQ(*e, (Decoded{DecodedCodePoint{source.skipBytes<1>(), CodePoint{'A'}}}));

    ASSERT_FALSE(++e);
}

TEST(utf8Decode, umlaut) {
    auto source = View{"\xc3\xbc"};

    auto e = utf8Decode(source);

    ASSERT_TRUE(e);
    ASSERT_TRUE(++e);
    ASSERT_EQ(*e, (Decoded{DecodedCodePoint{source, CodePoint{0xFC}}}));

    ASSERT_FALSE(++e);
}

TEST(utf8Decode, wrong) {
    auto source = View{
        "\xc3"
        "a"};

    auto e = utf8Decode(source);

    ASSERT_TRUE(e);
    ASSERT_TRUE(++e);
    ASSERT_EQ(*e, (Decoded{DecodedError{source.firstBytes<1>()}}));

    ASSERT_TRUE(++e);
    ASSERT_EQ(*e, (Decoded{DecodedCodePoint{source.skipBytes<1>(), CodePoint{'a'}}}));

    ASSERT_FALSE(++e);
}

TEST(utf8Decode, early) {
    auto source = View{
        "\xF0"
        ""};

    auto e = utf8Decode(source);

    ASSERT_TRUE(e);
    ASSERT_TRUE(++e);
    ASSERT_EQ(*e, (Decoded{DecodedError{source}}));

    ASSERT_FALSE(++e);
}
