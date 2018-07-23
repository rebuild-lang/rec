#pragma once
#include "File.h"
#include "Position.h"

#include "strings/View.h"

namespace text {

using View = strings::View;

struct TextRange {
    const File* file{};
    View text{};
    Position begin{};
    Position end{};

    bool operator==(const TextRange& o) const {
        return file == o.file && text.isContentEqual(o.text) && begin == o.begin && end == o.end;
    }
    bool operator!=(const TextRange& o) const { return !(*this == o); }
};

} // namespace text
