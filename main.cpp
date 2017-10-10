#include "scanner/tokenizer.h"

//#include <iostream>

int main() {
    auto f = scanner::file_t{strings::utf8_string{"TestFile"},
                             strings::utf8_string{"TestContent"}};
    auto d =
        scanner::tokenizer(scanner::tokenizer::config{scanner::column_t{8}});

    // std::cout << "Example = " << d.example() << '\n';
}
