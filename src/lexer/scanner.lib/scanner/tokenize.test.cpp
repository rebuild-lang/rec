#include <scanner/tokenize.h>

#include <gtest/gtest.h>

TEST(tokenize, basic) {
    using namespace scanner;
    using namespace text;

    auto input = String{"\n "};
    auto inputColon = String{":"};
    auto decoder = [&]() -> meta::CoEnumerator<DecodedPosition> {
        co_yield NewlinePosition{input, Position{Line{}, Column{}}};
        co_yield CodePointPosition{inputColon, CodePoint{':'}, Position{Line{2}, Column{2}}};
    }();
    auto tokGen = scanner::tokenize(std::move(decoder));
    tokGen++;
    ASSERT_TRUE(tokGen);
    const auto& tok = *tokGen;

    ASSERT_TRUE(tok.holds<NewLineIndentation>());

    const auto& lit = tok.get<NewLineIndentation>();
    EXPECT_EQ(input, strings::to_string(lit.input));
    EXPECT_EQ(1u, lit.position.line.v);
    EXPECT_EQ(1u, lit.position.column.v);

    tokGen++;
    ASSERT_TRUE(tokGen);
    const auto& tok2 = *tokGen;

    ASSERT_TRUE(tok2.holds<ColonSeparator>());
    const auto& lit2 = tok2.get<ColonSeparator>();
    EXPECT_EQ(inputColon, strings::to_string(lit2.input));
    EXPECT_EQ(2u, lit2.position.line.v);
    EXPECT_EQ(2u, lit2.position.column.v);

    tokGen++;
    ASSERT_FALSE(tokGen);
}
