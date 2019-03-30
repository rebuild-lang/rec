#include "rec/Compiler.h"

#include <iostream>

int main() {
    using namespace rec;

    auto config = ParserConfig{text::Column{8}};
    auto globals = instance::Scope{};
    globals.emplace(intrinsicAdapter::Adapter::moduleInstance<intrinsic::Rebuild>());

    // auto globalScope = instance::Scope{&globals};

    auto compiler = Compiler{config, std::move(globals)};

    // auto decode = [&](const auto& file) { return strings::utf8Decode(file.content); };
    // auto positions = [&](const auto& file) { return text::decodePosition(decode(file), config); };
    // auto tokenize = [&](const auto& file) { return scanner::tokenize(positions(file)); };
    // auto filter = [&](const auto& file) { return filter::filterTokens(tokenize(file)); };
    // auto blockify = [&](const auto& file) { return nesting::nestTokens(filter(file)); };

    auto file = text::File{
        strings::String{"TestFile"},
        strings::String{"\x07 \x00"
                        R"(# Rebuild.Context.declareVariable hif :Rebuild.literal.String = "Hello from Global!"

Rebuild.Context.declareFunction(() hi (a :Rebuild.literal.String) ():
    # Rebuild.say hif # TODO(arBmind): get globals working
    Rebuild.say "Hello from Hi"
    Rebuild.say a
end
hi "Hello from calling"

Rebuild.Context.declareVariable foo :Rebuild.literal.String = "Hello from Variable!"
Rebuild.say foo
hi foo

Rebuild.Context.declareModule test:
    Rebuild.say "parsing inside!"
end
)"}};

    // std::cout << "\nTokens:\n";
    //// for (auto t : tokenize(file)) std::cout << t << '\n';

    // std::cout << "\nBlocks:\n";
    // std::cout << blockify(file);

    std::cout << "\nCompile:\n";
    compiler.compile(file);

    // std::cout << "Example = " << d.example() << '\n';
}
