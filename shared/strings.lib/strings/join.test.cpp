#include "join.h"

#include "String.ostream.h"

#include <gtest/gtest.h>
#include <sstream>
#include <vector>

using strings::join;
using strings::String;

TEST(join, none) {
    auto ss = std::stringstream{};
    auto c = std::vector<String>{};
    auto d = String{";"};

    join(ss, c, d);

    ASSERT_EQ(ss.str(), std::string{});
}

TEST(join, one) {
    auto ss = std::stringstream{};
    auto c = std::vector{String{"1"}};
    auto d = String{";"};

    join(ss, c, d);

    ASSERT_EQ(ss.str(), std::string{"1"});
}

TEST(join, three) {
    auto ss = std::stringstream{};
    auto c = std::vector{String{"1"}, String{"2"}, String{"3"}};
    auto d = String{","};

    join(ss, c, d);

    ASSERT_EQ(ss.str(), std::string{"1,2,3"});
}
