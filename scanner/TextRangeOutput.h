#pragma once
#include "TextRange.h"

#include "strings/Output.h"

namespace scanner {

inline auto operator<<(std::ostream& out, const Position& p) -> std::ostream& {
    return out << '[' << p.line.v << ';' << p.column.v << ']';
}

inline auto operator<<(std::ostream& out, const TextRange& r) -> std::ostream& {
    out << r.begin << r.text << r.end << " in " << (r.file ? r.file->filename : String{"<null>"});
    return out;
}

} // namespace scanner
