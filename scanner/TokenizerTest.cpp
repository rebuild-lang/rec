#include "scanner/Tokenizer.h"

#include "gtest/gtest.h"

TEST(scanner, basic) {
    using namespace scanner;

    auto t = Tokenizer{Tokenizer::Config{Column{8}}};
    auto f = File{String{"testfile"}, String{"\n "}};
    auto tokGen = t.scanFile(f);
    ASSERT_TRUE(tokGen++);
    auto &tok = *tokGen;

    ASSERT_TRUE(tok.oneOf<NewLineIndentation>());
    ASSERT_TRUE(tok.range.text.isContentEqual(strings::View{"\n "}));
    ASSERT_EQ(1u, tok.range.begin.line.v);
    ASSERT_EQ(1u, tok.range.begin.column.v);
    ASSERT_EQ(2u, tok.range.end.line.v);
    ASSERT_EQ(2u, tok.range.end.column.v);
}
