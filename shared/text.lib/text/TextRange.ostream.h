#pragma once
#include "TextRange.h"

#include "Position.ostream.h"

#include "strings/String.ostream.h"
#include "strings/View.ostream.h"

namespace text {

inline auto operator<<(std::ostream& out, const TextRange& r) -> std::ostream& {
    return out << r.begin << r.text << r.end << " in " << (r.file != nullptr ? r.file->filename : String{"<null>"});
}

} // namespace text
