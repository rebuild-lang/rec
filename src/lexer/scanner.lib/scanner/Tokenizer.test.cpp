#include "scanner/Tokenizer.h"

#include "gtest/gtest.h"

TEST(scanner, basic) {
    using namespace scanner;
    using namespace text;

    auto t = Tokenizer{Tokenizer::Config{Column{8}}};
    auto f = File{String{"testfile"}, String{"\n "}};
    auto tokGen = t.scanFile(f);
    ASSERT_TRUE(tokGen++);
    const auto& tok = *tokGen;

    ASSERT_TRUE(tok.holds<NewLineIndentation>());

    const auto& lit = tok.get<NewLineIndentation>();
    ASSERT_TRUE(lit.range.text.isContentEqual(strings::View{"\n "}));
    ASSERT_EQ(1u, lit.range.begin.line.v);
    ASSERT_EQ(1u, lit.range.begin.column.v);
    ASSERT_EQ(2u, lit.range.end.line.v);
    ASSERT_EQ(2u, lit.range.end.column.v);
}
