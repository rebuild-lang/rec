#include "parser/token_preparation.h"

#include "gtest/gtest.h"

TEST(parser, token_preparation) {
    using namespace parser;

    auto tok_gen = prepare_tokens([]() -> meta::co_enumerator<token_data> {
        co_yield token_data{{}, scanner::comment_literal{}};
    }());
    ASSERT_TRUE(static_cast<bool>(tok_gen));
}
