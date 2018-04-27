#include "parser/block/Parser.h"
#include "parser/expression/Parser.h"
#include "parser/filter/Parser.h"
#include "scanner/Tokenizer.h"

#include "api/Context.h"
#include "intrinsic/Adapter.h"

#include "parser/block/TokenOutput.h"
#include "scanner/TokenOutput.h"

#include <iostream>

int main() {
    auto config = scanner::Tokenizer::Config{scanner::Column{8}};
    auto scope = instance::Scope{};
    scope.emplace(intrinsicAdapter::Adapter::moduleInstance<intrinsic::Rebuild>());

    auto tokenize = [&](const auto& file) { return scanner::Tokenizer(config).scanFile(file); };
    auto filter = [&](const auto& file) { return parser::filter::Parser::parse(tokenize(file)); };
    auto blockify = [&](const auto& file) { return parser::block::Parser::parse(filter(file)); };
    auto parse = [&](const auto& file) { return parser::expression::Parser::parse(blockify(file), scope); };

    auto file = scanner::File{strings::String{"TestFile"}, strings::String{R"(
Rebuild.say "Hello!"
)"}};

    std::cout << "\nTokens:\n";
    for (auto t : tokenize(file)) std::cout << t << '\n';

    std::cout << "\nBlocks:\n";
    std::cout << blockify(file);

    std::cout << "\nParse:\n";
    parse(file);

    // std::cout << "Example = " << d.example() << '\n';
}
