#pragma once
#include "strings/CodePoint.h"

#include <vector>

namespace strings {

/// naively strong typed index
struct Index {
    uint32_t v;
};

/// owning utf8 encoded readonly string
// all useful methods are on View !
struct String {
    using This = String;
    using Data = uint8_t;

private:
    std::vector<Data> m;

public:
    String() = default; // valid empty string

    String(std::vector<Data> &&src) noexcept // take data from existing vector
        : m(std::move(src)) {}
    // explicit String(const char* str) : data_m(str, str + strlen(str)) {}

    explicit String(std::initializer_list<Data> il) // from initializer list
        : m(il) {}
    explicit String(std::initializer_list<char> il)
        : m(reinterpret_cast<std::initializer_list<Data> &>(il)) {}

    template<size_t N>
    explicit String(const char (&str)[N]) // init from string literal
        : m(str, str + N - 1) {}

    String(const Data *b, const Data *e)
        : m(b, e) {}

    const Data *data() const { return m.data(); }
    Count byteCount() const { return {static_cast<uint32_t>(m.size())}; }
    bool isEmpty() const { return m.empty(); }

    auto begin() const { return data(); }
    auto end() const { return data() + byteCount().v; }

    bool operator==(const This &o) const { return m == o.m; }
    bool operator<(const This &o) const { return m < o.m; }
};

} // namespace strings
