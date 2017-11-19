#include "parser/token_preparation.h"

#include "parser/prepared_token_builder.h"
#include "scanner/token_builder.h"

#include "gtest/gtest.h"

#include <vector>

using namespace parser;
using namespace parser::prepared;

using scan_tokens = std::vector<scan_token>;
using prep_tokens = std::vector<token>;

struct tokens_transform_data {
    const char *name;
    scan_tokens input;
    prep_tokens expected;

    tokens_transform_data(const char *name)
        : name{name} {}
    tokens_transform_data(const tokens_transform_data &) = default;
    tokens_transform_data &operator=(const tokens_transform_data &) & = default;
    tokens_transform_data(tokens_transform_data &&) = default;
    tokens_transform_data &operator=(tokens_transform_data &&) & = default;

    template<class... Tok>
    auto in(Tok &&... tok) && -> tokens_transform_data {
        input = scanner::build_tokens(std::forward<Tok>(tok)...);
        return *this;
    }
    template<class... Tok>
    auto out(Tok &&... tok) && -> tokens_transform_data {
        expected = parser::prepared::build_tokens(std::forward<Tok>(tok)...);
        return *this;
    }
};
static auto operator<<(std::ostream &out, const tokens_transform_data &ttd) -> std::ostream & {
    out << "name: " << ttd.name << "\n";
    out << "input:\n";
    for (auto &t : ttd.input) out << t << '\n';
    out << "expected:\n";
    for (auto &t : ttd.expected) out << t << '\n';
    return out;
}

class token_transformations : public testing::TestWithParam<tokens_transform_data> {};

TEST_P(token_transformations, token_preparation) {
    tokens_transform_data data = GetParam();
    auto input = [&]() -> meta::co_enumerator<scan_token> {
        for (const auto &t : data.input) {
            co_yield t;
        }
    }();

    auto tok_gen = token_preparation::prepare(input);

    for (const auto &et : data.expected) {
        tok_gen++;
        ASSERT_TRUE(static_cast<bool>(tok_gen));
        const auto &tok = *tok_gen;
        ASSERT_EQ(tok.data, et.data);
    }
}

INSTANTIATE_TEST_CASE_P(
    filter_start,
    token_transformations,
    ::testing::Values(
        tokens_transform_data("Filter out starting comment")
            .in(scanner::comment_literal{}, new_line_indentation{}, view_t{})
            .out(new_line_indentation{}, id().both_separated()),
        tokens_transform_data("Filter out starting indented comment")
            .in(new_line_indentation{}, scanner::comment_literal{}, new_line_indentation{}, view_t{})
            .out(new_line_indentation{}, id().both_separated()),
        tokens_transform_data("Filter out starting comment whitespace comment")
            .in(new_line_indentation{},
                scanner::comment_literal{},
                scanner::white_space_separator{},
                scanner::comment_literal{},
                new_line_indentation{},
                view_t{})
            .out(new_line_indentation{}, id().both_separated()),
        tokens_transform_data("Filter multiple newlines")
            .in(new_line_indentation{}, new_line_indentation{}, view_t{})
            .out(new_line_indentation{}, id().both_separated()),
        tokens_transform_data("Filter even more newlines")
            .in(new_line_indentation{}, new_line_indentation{}, new_line_indentation{}, view_t{})
            .out(new_line_indentation{}, id().both_separated()) //
        ));

INSTANTIATE_TEST_CASE_P(
    filter_end,
    token_transformations,
    ::testing::Values(
        tokens_transform_data("Filter out final comment")
            .in(new_line_indentation{}, view_t{}, scanner::comment_literal{})
            .out(new_line_indentation{}, id().both_separated()),
        tokens_transform_data("Filter out final whitespace")
            .in(new_line_indentation{}, view_t{}, scanner::white_space_separator{})
            .out(new_line_indentation{}, id().both_separated()),
        tokens_transform_data("Filter out final newline")
            .in(new_line_indentation{}, view_t{}, new_line_indentation{})
            .out(new_line_indentation{}, id().both_separated()) //
        ));

INSTANTIATE_TEST_CASE_P(
    blocks,
    token_transformations,
    ::testing::Values(
        tokens_transform_data("Mutate identifier block start")
            .in(new_line_indentation{}, view_t{"begin"}, colon_separator{}, new_line_indentation{})
            .out(new_line_indentation{}, id("begin").both_separated(), block_start_indentation{}),

        tokens_transform_data("Mutate identifier block start with comment")
            .in(view_t{"begin"},
                colon_separator{},
                scanner::white_space_separator{},
                scanner::comment_literal{},
                new_line_indentation{})
            .out(new_line_indentation{}, id("begin").both_separated(), block_start_indentation{}),

        tokens_transform_data("Mutate block end")
            .in(new_line_indentation{},
                colon_separator{},
                new_line_indentation{},
                view_t{"end"},
                new_line_indentation{})
            .out(new_line_indentation{}, block_start_indentation{}, block_end_indentation{}) //
        ));

INSTANTIATE_TEST_CASE_P(
    neighbors,
    token_transformations,
    ::testing::Values(
        tokens_transform_data("With white spaces")
            .in(scanner::white_space_separator{},
                view_t{"left"},
                view_t{"middle"},
                view_t{"right"},
                scanner::white_space_separator{},
                view_t{"free"},
                scanner::white_space_separator{})
            .out(
                new_line_indentation{},
                id("left").left_separated(),
                id("middle"),
                id("right").right_separated(),
                id("free").both_separated()),

        tokens_transform_data("border cases")
            .in(view_t{"left"}, view_t{"right"})
            .out(new_line_indentation{}, id("left").left_separated(), id("right").right_separated()),

        tokens_transform_data("Brackets")
            .in(bracket_open{}, view_t{"left"}, view_t{"right"}, bracket_close{}, view_t{"stuck"}, bracket_open{})
            .out(
                new_line_indentation{},
                bracket_open{},
                id("left").left_separated(),
                id("right").right_separated(),
                bracket_close{},
                id("stuck"),
                bracket_open{}),

        tokens_transform_data("Comma")
            .in(scanner::white_space_separator{}, view_t{"left"}, comma_separator{}, view_t{"right"})
            .out(new_line_indentation{}, id("left").both_separated(), comma_separator{}, id("right").both_separated()),

        tokens_transform_data("Semicolon")
            .in(scanner::white_space_separator{}, view_t{"left"}, semicolon_separator{}, view_t{"right"})
            .out(
                new_line_indentation{},
                id("left").both_separated(),
                semicolon_separator{},
                id("right").both_separated()) //
        ));
