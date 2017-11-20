#pragma once
#include "strings/code_point.h"

#include <ostream>
#include <vector>

namespace strings {

/// naively strong typed index
struct index_t {
    uint32_t v;
};

/// owning utf8 encoded readonly string
// all useful methods are on utf8_view !
struct utf8_string {
    using this_t = utf8_string;
    using value_type = uint8_t;

    utf8_string() = default; // valid empty string

    utf8_string(std::vector<value_type> &&src) noexcept // take data from existing vector
        : data_m(std::move(src)) {}
    // explicit utf8_string(const char* str) : data_m(str, str + strlen(str)) {}

    explicit utf8_string(std::initializer_list<value_type> il) // from initializer list
        : data_m(il) {}
    explicit utf8_string(std::initializer_list<char> il)
        : data_m(reinterpret_cast<std::initializer_list<value_type> &>(il)) {}

    template<size_t N>
    explicit utf8_string(const char (&str)[N]) // init from string literal
        : data_m(str, str + N - 1) {}

    utf8_string(const value_type *b, const value_type *e)
        : data_m(b, e) {}

    // full value semantics
    utf8_string(const this_t &) = default;
    this_t &operator=(const this_t &) = default;
    utf8_string(this_t &&) = default;
    this_t &operator=(this_t &&) = default;

    const value_type *data() const { return data_m.data(); }
    count_t byte_count() const { return {static_cast<uint32_t>(data_m.size())}; }
    bool is_empty() const { return data_m.empty(); }

    auto begin() const { return data(); }
    auto end() const { return data() + byte_count().v; }

    bool operator==(const this_t &o) const { return data_m == o.data_m; }
    bool operator<(const this_t &o) const { return data_m < o.data_m; }

private:
    std::vector<value_type> data_m;
};

inline std::ostream &operator<<(std::ostream &o, const utf8_string &s) {
    for (auto c : s) o << static_cast<char>(c);
    return o;
}

} // namespace strings
