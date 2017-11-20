#pragma once
#include "strings/utf8_string.h"

namespace instance {

using name_t = strings::utf8_string;

struct type_t {
    name_t name;
    uint64_t size{};
    // TODO
};
using type_ptr = const type_t *;

inline auto name_of(const type_t &type) -> const name_t & { return type.name; }

inline auto operator<<(std::ostream &out, const type_t &t) -> std::ostream & {
    return out << ":" << t.name << "(bytes: " << t.size << ")";
}

} // namespace instance
