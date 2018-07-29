#include "Unreachable.h"

#include <assert.h>
#include <cstdlib>
#include <iostream>

void meta::details::reportUnreachable() {
    std::cerr << "reached unreachable code";
    assert(false); // code should not be reached
    std::abort();
}
