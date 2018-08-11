#pragma once
#include "File.h"
#include "Position.h"

#include "strings/View.h"

namespace text {

using View = strings::View;

struct Range {
    const File* file{};
    View view{};
    Position begin{};
    Position end{};

    bool operator==(const Range& o) const {
        return file == o.file && view.isContentEqual(o.view) && begin == o.begin && end == o.end;
    }
    bool operator!=(const Range& o) const { return !(*this == o); }
};

} // namespace text
