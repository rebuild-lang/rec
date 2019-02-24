#pragma once
#include "Counter.h"

#include "meta/Optional.h"

#include <initializer_list>
#include <string>
#include <vector>

namespace strings {

/// owning utf8 encoded readonly string
// all useful methods are on View !
struct String {
    using This = String;
    using Char = char;
    static_assert(sizeof(Char) == sizeof(uint8_t), "char is not a byte");

private:
    std::vector<Char> m;

public:
    String() = default; // valid empty string
    String(const This&) = default;
    String(This&&) noexcept = default;
    String& operator=(const This&) & = default;
    String& operator=(This&&) & noexcept = default;
    ~String() = default;

    explicit String(std::vector<Char>&& src) noexcept // take data from existing vector
        : m(std::move(src)) {}

    String(std::initializer_list<Char> il) // from initializer list
        : m(il) {}

    template<size_t N>
    explicit String(const char (&str)[N]) // init from string literal
        : m(str, str + N - 1) {}

    String(const Char* b, const Char* e)
        : m(b, e) {}

    explicit operator std::string() const { return {begin(), end()}; }

    auto data() const -> const Char* { return m.data(); }
    auto byteCount() const -> Counter { return {m.size()}; }
    bool isEmpty() const { return m.empty(); }

    auto begin() const -> const Char* { return m.data(); }
    auto end() const -> const Char* { return m.data() + m.size(); }

    bool operator==(const This& o) const { return m == o.m; }
    bool operator<(const This& o) const { return m < o.m; }
};
using OptionalString = meta::Optional<meta::DefaultPacked<String>>;

} // namespace strings
