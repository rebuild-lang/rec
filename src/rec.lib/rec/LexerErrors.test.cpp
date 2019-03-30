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

input line: 1
\[80] \[e280]x
~~~~~ ~~~~~~~

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

input line: 1
\[7] \[0]
~~~~ ~~~~

)"s;
    EXPECT_EQ(outbuffer.str(), expected);
}
