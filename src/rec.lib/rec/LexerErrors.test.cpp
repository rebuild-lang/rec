#include "Compiler.h"

#include "gtest/gtest.h"

#include <sstream>

using namespace rec;
using namespace std::string_literals;

auto makeTestCompiler(std::stringstream& out) -> Compiler {
    auto config = Config{text::Column{8}};
    config.diagnosticsOutput = &out;
    return Compiler{config};
}

TEST(LexerErrors, decodeErrors) {
    auto outbuffer = std::stringstream{};
    auto compiler = makeTestCompiler(outbuffer);

    auto file = text::File{strings::String{"TestFile"}, strings::String{"\x80 \xE2\x80x"}};
    compiler.compile(file);

    auto expected = R"(1 diagnostics:
>>> rebuild-lexer[1]: Invalid UTF8 Encoding

The UTF8-decoder encountered multiple invalid encodings

1 |\[80] \[e280]x
  |~~~~~ ~~~~~~~


)"s;
    EXPECT_EQ(outbuffer.str(), expected);
}

TEST(LexerErrors, unexpectedCharacter) {
    auto outbuffer = std::stringstream{};
    auto compiler = makeTestCompiler(outbuffer);

    auto file = text::File{strings::String{"TestFile"}, strings::String{"\x07 \x00"}};
    compiler.compile(file);

    auto expected = R"(1 diagnostics:
>>> rebuild-lexer[2]: Unexpected characters

The tokenizer encountered multiple characters that are not part of any Rebuild language token.

1 |\[7] \0
  |~~~~ ~~


)"s;
    EXPECT_EQ(outbuffer.str(), expected);
}

TEST(LexerErrors, mixedIndentation) {
    auto outbuffer = std::stringstream{};
    auto compiler = makeTestCompiler(outbuffer);

    auto file = text::File{strings::String{"TestFile"}, strings::String{"\n \n\t\n"}};
    compiler.compile(file);

    auto expected = R"(1 diagnostics:
>>> rebuild-lexer[3]: Mixed Indentation Characters

The indentation mixes tabs and spaces.

1 | \n
2 |\t
  |~~


)"s;
    EXPECT_EQ(outbuffer.str(), expected);
}

TEST(LexerErrors, indentationDecodeError) {
    auto outbuffer = std::stringstream{};
    auto compiler = makeTestCompiler(outbuffer);

    auto file = text::File{strings::String{"TestFile"}, strings::String{"\n \n\x80\n"}};
    compiler.compile(file);

    auto expected = R"(1 diagnostics:
>>> rebuild-lexer[1]: Invalid UTF8 Encoding

The UTF8-decoder encountered an invalid encoding

1 | \n
2 |\[80]
  |~~~~~


)"s;
    EXPECT_EQ(outbuffer.str(), expected);
}

TEST(LexerErrors, unknownEscapeSequence) {
    auto outbuffer = std::stringstream{};
    auto compiler = makeTestCompiler(outbuffer);

    auto file = text::File{strings::String{"TestFile"}, strings::String{R"("\?")"}};
    compiler.compile(file);

    auto expected = R"(1 diagnostics:
>>> rebuild-lexer[11]: Unkown escape sequence

These Escape sequences are unknown.

1 |"\?"
  | ~~


)"s;
    EXPECT_EQ(outbuffer.str(), expected);
}

TEST(LexerErrors, numberMissingBoundary) {
    auto outbuffer = std::stringstream{};
    auto compiler = makeTestCompiler(outbuffer);

    auto file = text::File{strings::String{"TestFile"}, strings::String{"3.14p"}};
    compiler.compile(file);

    auto expected = R"(1 diagnostics:
>>> rebuild-lexer[22]: Missing boundary

The number literal ends with an unknown suffix.

1 |3.14p
  |    ~


)"s;
    EXPECT_EQ(outbuffer.str(), expected);
}

TEST(LexerErrors, operatorParts) {
    auto outbuffer = std::stringstream{};
    auto compiler = makeTestCompiler(outbuffer);

    auto file = text::File{strings::String{"TestFile"}, strings::String{"*›‹\n"}};
    compiler.compile(file);

    auto expected = R"(2 diagnostics:
>>> rebuild-lexer[31]: Operator unexpected close

There was no opening sign before the closing sign.

1 |*›‹
  | ~


>>> rebuild-lexer[32]: Operator not closed

The operator ends before the closing sign was found.

1 |*›‹
  |  ~


)"s;
    EXPECT_EQ(outbuffer.str(), expected);
}

TEST(LexerErrors, unexpectedColon) {
    auto outbuffer = std::stringstream{};
    auto compiler = makeTestCompiler(outbuffer);

    auto file = text::File{strings::String{"TestFile"}, strings::String{"\n:\n"}};
    compiler.compile(file);

    auto expected = R"(1 diagnostics:
>>> rebuild-lexer[4]: Unexpected colon

The colon cannot be the only token on a line.

2 |:
  |~


)"s;
    EXPECT_EQ(outbuffer.str(), expected);
}

TEST(LexerErrors, unexpectedIndent) {
    auto outbuffer = std::stringstream{};
    auto compiler = makeTestCompiler(outbuffer);

    auto file = text::File{strings::String{"TestFile"}, strings::String{"\n   a\n  c"}};
    compiler.compile(file);

    auto expected = R"(1 diagnostics:
>>> rebuild-lexer[5]: Unexpected indent

The indentation is above the regular block level, but does not leave the block.

2 |
3 |  c
  |~~


)"s;
    EXPECT_EQ(outbuffer.str(), expected);
}

TEST(LexerErrors, unexpectedTokenAfterEnd) {
    auto outbuffer = std::stringstream{};
    auto compiler = makeTestCompiler(outbuffer);

    auto file = text::File{strings::String{"TestFile"}, strings::String{"b:\nend+ nx"}};
    compiler.compile(file);

    auto expected = R"(1 diagnostics:
>>> rebuild-lexer[6]: Unexpected tokens after end

After end no more tokens are allowed.

2 |b:
3 |end+ nx
  |   ~ ~~


)"s;
    EXPECT_EQ(outbuffer.str(), expected);
}

TEST(LexerErrors, unexpectedBlockEnd) {
    auto outbuffer = std::stringstream{};
    auto compiler = makeTestCompiler(outbuffer);

    auto file = text::File{strings::String{"TestFile"}, strings::String{"b\n end"}};
    compiler.compile(file);

    auto expected = R"(1 diagnostics:
>>> rebuild-lexer[7]: Unexpected block end

The end keyword is only allowed to end blocks

2 |b
3 | end
  | ~~~


)"s;
    EXPECT_EQ(outbuffer.str(), expected);
}
