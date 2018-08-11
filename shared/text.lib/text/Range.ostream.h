#pragma once
#include "Range.h"

#include "Position.ostream.h"

#include "strings/String.ostream.h"
#include "strings/View.ostream.h"

namespace text {

inline auto operator<<(std::ostream& out, const Range& r) -> std::ostream& {
    return out << r.begin << r.view << r.end << " in " << (r.file != nullptr ? r.file->filename : String{"<null>"});
}

} // namespace text
