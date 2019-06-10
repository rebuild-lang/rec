#pragma once
#include "strings/String.h"

namespace text {

using String = strings::String;

struct File {
    String filename{};
    String content{};
};

} // namespace text
