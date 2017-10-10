#pragma once
#include "strings/code_point.h"

#include <vector>

namespace strings {

/// naively strong typed index
struct index_t {
    uint32_t v;
};

/// owning utf8 encoded readonly string
// all useful methods are on utf8_view !
struct utf8_string {
    using value_type = uint8_t;

    utf8_string() = default; // valid empty string
    utf8_string(const utf8_string &) = delete;
    utf8_string &operator=(const utf8_string &) = delete;
    utf8_string(utf8_string &&) = default;
    utf8_string &operator=(utf8_string &&) = default;

    utf8_string(std::vector<value_type> &&src)
        : data_m(std::move(src)) {}
    // explicit utf8_string(const char* str) : data_m(str, str + strlen(str)) {}

    explicit utf8_string(std::initializer_list<value_type> il)
        : data_m(il) {}
    explicit utf8_string(std::initializer_list<char> il)
        : data_m(reinterpret_cast<std::initializer_list<value_type> &>(il)) {}

    template<size_t N>
    explicit utf8_string(const char (&str)[N])
        : data_m(str, str + N - 1) {}

    const value_type *data() const { return data_m.data(); }
    count_t byte_count() const { return {static_cast<uint32_t>(data_m.size())}; }

    auto begin() const { return data_m.begin(); }
    auto end() const { return data_m.end(); }

  private:
    std::vector<value_type> data_m;
};

} // namespace strings
