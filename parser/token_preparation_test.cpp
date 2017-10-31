#include "parser/token_preparation.h"

#include "gtest/gtest.h"

#include <vector>

using namespace scanner;
using namespace parser;

using tokens = std::vector<token>;

struct id_builder {
    identifier_literal lit;

    id_builder left_separated() && {
        lit.left_separated = true;
        return *this;
    }
    id_builder right_separated() && {
        lit.right_separated = true;
        return *this;
    }
    id_builder both_separated() && {
        lit.left_separated = true;
        lit.right_separated = true;
        return *this;
    }
    template<size_t N>
    id_builder text(const char (&text)[N]) && {
        lit.content += view_t{text};
        return *this;
    }

    identifier_literal id() && { return lit; }
    operator_literal op() && {
        auto op_lit = operator_literal{};
        op_lit.content = std::move(lit.content);
        op_lit.splitted_from = std::move(lit.splitted_from);
        op_lit.left_separated = lit.left_separated;
        op_lit.right_separated = lit.right_separated;
        return op_lit;
    }
};
auto id() -> id_builder { return {}; }

template<size_t N>
auto id(const char (&text)[N]) -> id_builder {
    return id_builder{}.text(text);
}

template<class Tok>
struct fake_token {
    static token build(Tok &&t) {
        auto tok = token{};
        tok.data = std::move(t);
        return tok;
    }
};
template<>
struct fake_token<token> {
    static token build(token &&t) { return std::move(t); }
};
template<>
struct fake_token<id_builder> {
    static token build(id_builder &&b) {
        auto tok = token{};
        tok.data = std::move(b).id();
        return tok;
    }
};

template<class... Tok>
tokens fake_tokens(Tok &&... t) {
    tokens result;
    (void)std::initializer_list<int>{(result.push_back(fake_token<Tok>::build(std::forward<Tok>(t))), 0)...};
    return result;
}

struct tokens_transform_data {
    const char *name;
    std::vector<token> input;
    std::vector<token> expected;

    tokens_transform_data(const char *name)
        : name{name} {}
    tokens_transform_data(const tokens_transform_data &) = default;
    tokens_transform_data &operator=(const tokens_transform_data &) = default;
    tokens_transform_data(tokens_transform_data &&) = default;
    tokens_transform_data &operator=(tokens_transform_data &&) = default;

    tokens_transform_data in(tokens &&t) && {
        input = std::move(t);
        return *this;
    }
    tokens_transform_data out(tokens &&t) && {
        expected = std::move(t);
        return *this;
    }
};
std::ostream &operator<<(std::ostream &out, const tokens_transform_data &ttd) {
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
    auto input = [&]() -> meta::co_enumerator<token> {
        for (const auto &t : data.input) {
            co_yield t;
        }
    }();

    auto tok_gen = prepare_tokens(input);

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
            .in(fake_tokens(comment_literal{}, new_line_indentation{}, identifier_literal{}))
            .out(fake_tokens(new_line_indentation{}, id().both_separated())),
        tokens_transform_data("Filter out starting indented comment")
            .in(fake_tokens(new_line_indentation{}, comment_literal{}, new_line_indentation{}, identifier_literal{}))
            .out(fake_tokens(new_line_indentation{}, id().both_separated())),
        tokens_transform_data("Filter out starting comment whitespace comment")
            .in(fake_tokens(
                new_line_indentation{},
                comment_literal{},
                white_space_separator{},
                comment_literal{},
                new_line_indentation{},
                identifier_literal{}))
            .out(fake_tokens(new_line_indentation{}, id().both_separated())),
        tokens_transform_data("Filter multiple newlines")
            .in(fake_tokens(new_line_indentation{}, new_line_indentation{}, identifier_literal{}))
            .out(fake_tokens(new_line_indentation{}, id().both_separated())),
        tokens_transform_data("Filter even more newlines")
            .in(fake_tokens(
                new_line_indentation{}, new_line_indentation{}, new_line_indentation{}, identifier_literal{}))
            .out(fake_tokens(new_line_indentation{}, id().both_separated())) //
        ));

INSTANTIATE_TEST_CASE_P(
    filter_end,
    token_transformations,
    ::testing::Values(
        tokens_transform_data("Filter out final comment")
            .in(fake_tokens(new_line_indentation{}, identifier_literal{}, comment_literal{}))
            .out(fake_tokens(new_line_indentation{}, id().both_separated())),
        tokens_transform_data("Filter out final whitespace")
            .in(fake_tokens(new_line_indentation{}, identifier_literal{}, white_space_separator{}))
            .out(fake_tokens(new_line_indentation{}, id().both_separated())),
        tokens_transform_data("Filter out final newline")
            .in(fake_tokens(new_line_indentation{}, identifier_literal{}, new_line_indentation{}))
            .out(fake_tokens(new_line_indentation{}, id().both_separated())) //
        ));

INSTANTIATE_TEST_CASE_P(
    blocks,
    token_transformations,
    ::testing::Values(
        tokens_transform_data("Mutate identifier block start")
            .in(fake_tokens(new_line_indentation{}, id("begin"), colon_separator{}, new_line_indentation{}))
            .out(fake_tokens(new_line_indentation{}, id("begin").both_separated(), block_start_indentation{})),

        tokens_transform_data("Mutate identifier block start with comment")
            .in(fake_tokens(
                id("begin"), colon_separator{}, white_space_separator{}, comment_literal{}, new_line_indentation{}))
            .out(fake_tokens(new_line_indentation{}, id("begin").both_separated(), block_start_indentation{})),

        tokens_transform_data("Mutate block end")
            .in(fake_tokens(
                new_line_indentation{}, colon_separator{}, new_line_indentation{}, id("end"), new_line_indentation{}))
            .out(fake_tokens(new_line_indentation{}, block_start_indentation{}, block_end_indentation{})) //
        ));

INSTANTIATE_TEST_CASE_P(
    neighbors,
    token_transformations,
    ::testing::Values(
        tokens_transform_data("With white spaces")
            .in(fake_tokens(
                white_space_separator{},
                id("left"),
                id("middle"),
                id("right"),
                white_space_separator{},
                id().text("free").id(),
                white_space_separator{}))
            .out(fake_tokens(
                new_line_indentation{},
                id("left").left_separated(),
                id("middle"),
                id("right").right_separated(),
                id("free").both_separated())),

        tokens_transform_data("border cases")
            .in(fake_tokens(id("left"), id("right")))
            .out(fake_tokens(new_line_indentation{}, id("left").left_separated(), id("right").right_separated())),

        tokens_transform_data("Brackets")
            .in(fake_tokens(bracket_open{}, id("left"), id("right"), bracket_close{}, id("stuck"), bracket_open{}))
            .out(fake_tokens(
                new_line_indentation{},
                bracket_open{},
                id("left").left_separated(),
                id("right").right_separated(),
                bracket_close{},
                id("stuck"),
                bracket_open{})),

        tokens_transform_data("Comma")
            .in(fake_tokens(white_space_separator{}, id("left"), comma_separator{}, id("right")))
            .out(fake_tokens(
                new_line_indentation{}, id("left").both_separated(), comma_separator{}, id("right").both_separated())),

        tokens_transform_data("Semicolon")
            .in(fake_tokens(white_space_separator{}, id("left"), semicolon_separator{}, id("right")))
            .out(fake_tokens(
                new_line_indentation{},
                id("left").both_separated(),
                semicolon_separator{},
                id("right").both_separated())) //
        ));
