#pragma once
#include "File.h"
#include "Position.h"

#include "strings/View.h"

namespace text {

using strings::View;

struct Range {
    using This = Range;

    const File* file{};
    View view{};
    Position begin{};
    Position end{};

    constexpr bool operator==(const This& o) const {
        return file == o.file && view.isContentEqual(o.view) && begin == o.begin && end == o.end;
    }
    constexpr bool operator!=(const This& o) const { return !(*this == o); }
};

} // namespace text
