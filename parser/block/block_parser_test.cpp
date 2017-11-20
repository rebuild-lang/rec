#include "parser/block/block_parser.h"

#include "parser/block/block_token_builder.h"
#include "parser/filter/filter_token_builder.h"

#include "gtest/gtest.h"

using namespace parser;
using namespace parser::block;

using filt_tokens = std::vector<filter_token>;

struct grouping_transform_data {
    const char *name;
    filt_tokens input;
    block_literal expected;

    grouping_transform_data(const char *name)
        : name{name} {}
    grouping_transform_data(const grouping_transform_data &) = default;
    grouping_transform_data &operator=(const grouping_transform_data &) = default;
    grouping_transform_data(grouping_transform_data &&) = default;
    grouping_transform_data &operator=(grouping_transform_data &&) = default;

    template<class... Tok>
    auto in(Tok &&... tok) && -> grouping_transform_data {
        input = filter::build_tokens(std::forward<Tok>(tok)...);
        return *this;
    }
    template<class... Lines>
    auto out(Lines &&... lines) && -> grouping_transform_data {
        expected = block_literal{{std::forward<Lines>(lines)...}};
        return *this;
    }
};
static auto operator<<(std::ostream &out, const grouping_transform_data &ttd) -> std::ostream & {
    out << "name: " << ttd.name << "\n";
    out << "input:\n";
    for (auto &t : ttd.input) out << t << '\n';
    out << "expected:\n";
    out << ttd.expected << '\n';
    return out;
}

class grouping_transformations : public testing::TestWithParam<grouping_transform_data> {};

TEST_P(grouping_transformations, block_parser) {
    grouping_transform_data data = GetParam();
    auto input = [&]() -> meta::co_enumerator<filter_token> {
        for (const auto &t : data.input) {
            co_yield t;
        }
    }();

    auto blk = block::parser::parse(input);

    ASSERT_EQ(blk, data.expected);
}

INSTANTIATE_TEST_CASE_P(
    examples,
    grouping_transformations,
    ::testing::Values( //
        grouping_transform_data("Example")
            .in(filter::id("if"),
                filter::id("a"),
                new_line(column_t{4}),
                filter::op("&&"),
                filter::id("b"),
                block_start(column_t{4}),
                filter::id("print_a"),
                new_line(column_t{4}),
                filter::id("print_b"),
                new_line(column_t{1}),
                filter::id("else"),
                block_start(column_t{4}),
                filter::id("print_c"),
                block_end(column_t{1}))
            .out(build_tokens(
                id("if"),
                id("a"),
                op("&&"),
                id("b"),
                blk(column_t{4}, build_tokens(id("print_a")), build_tokens(id("print_b"))),
                id("else"),
                blk(column_t{4}, build_tokens(id("print_c"))))) //
        ));
