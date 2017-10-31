#include "scanner/tokenizer.h"

#include "gtest/gtest.h"

TEST(scanner, basic) {
    using namespace scanner;

    auto t = tokenizer{tokenizer::config{column_t{8}}};
    auto f = file_t{string_t{"testfile"}, string_t{"\n "}};
    auto tok_gen = t.scan_file(f);
    ASSERT_TRUE(tok_gen++);
    auto &tok = *tok_gen;

    ASSERT_TRUE(tok.one_of<new_line_indentation>());
    ASSERT_TRUE(tok.range.text.content_equals(strings::utf8_view{"\n "}));
    ASSERT_EQ(1u, tok.range.begin_position.line.v);
    ASSERT_EQ(1u, tok.range.begin_position.column.v);
    ASSERT_EQ(2u, tok.range.end_position.line.v);
    ASSERT_EQ(2u, tok.range.end_position.column.v);
}
