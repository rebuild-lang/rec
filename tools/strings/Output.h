#pragma once

#include "Rope.h"
#include "String.h"
#include "View.h"

#include <ostream>

namespace strings {

inline std::ostream& operator<<(std::ostream& o, const String& s) {
    for (auto c : s) o << static_cast<char>(c);
    return o;
}

inline std::ostream& operator<<(std::ostream& out, const View& v) { return out << std::string(v.begin(), v.end()); }

inline std::ostream& operator<<(std::ostream& out, const Rope& r) { return out << to_string(r); }

} // namespace strings
