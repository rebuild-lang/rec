#include "Unreachable.h"

#include <cassert>
#include <cstdlib>
#include <iostream>

[[noreturn]] void meta::details::reportUnreachable() {
    std::cerr << "reached unreachable code";
    assert(false); // code should not be reached
    std::abort();
}
