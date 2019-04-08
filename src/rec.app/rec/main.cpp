#include "rec/Compiler.h"

#include <iostream>

#ifdef _WIN32
#    include <Windows.h>
#endif

int main() {

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    using namespace rec;

    auto config = Config{text::Column{8}};
    // config.tokenOutput = &std::cout;
    // config.blockOutput = &std::cout;
    config.diagnosticsOutput = &std::cout;

    auto compiler = Compiler{config};

    auto file = text::File{
        strings::String{"TestFile"},
        strings::String{"\n: "
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

    compiler.compile(file);
}
