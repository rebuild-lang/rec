#include "scanner/Tokenizer.h"

//#include <iostream>

int main() {
    auto f = scanner::File{strings::String{"TestFile"}, strings::String{"TestContent"}};
    auto d = scanner::Tokenizer(scanner::Tokenizer::Config{scanner::Column{8}});

    // std::cout << "Example = " << d.example() << '\n';
}
