#pragma once
#include "strings/code_point.h"
#include "strings/utf8_string.h"
#include "strings/utf8_view.h"

#include "meta/algorithm.h"
#include "meta/overloaded.h"
#include "meta/variant.h"

#include <ostream>
#include <vector>

namespace strings {

/// representation of a piecewise string
// use this to efficiently build large strings
struct rope {
    using this_t = rope;
    using element_t = meta::variant<code_point_t, utf8_string, utf8_view>;

    rope() = default; // valid empty rope

    // full value semantics
    rope(const this_t &) = default;
    this_t &operator=(const this_t &) = default;
    rope(this_t &&) = default;
    this_t &operator=(this_t &&) = default;

    explicit rope(const utf8_view &v) { data_m.emplace_back(v); }

    // append operators
    this_t &operator+=(code_point_t c) {
        data_m.emplace_back(c);
        return *this;
    }
    this_t &operator+=(utf8_string &&s) {
        if (!s.is_empty()) data_m.emplace_back(std::move(s));
        return *this;
    }
    this_t &operator+=(utf8_view v) {
        if (!v.is_empty()) data_m.emplace_back(v);
        return *this;
    }

    count_t byte_count() const {
        return meta::accumulate(data_m, count_t{0}, [](count_t c, const element_t &e) {
            return e.visit(
                [=](code_point_t cp) { return c + cp.utf8_byte_count(); },
                [=](const utf8_string &s) { return c + s.byte_count(); },
                [=](const utf8_view &v) { return c + v.byte_count(); });
        });
    }
    bool is_empty() const { return data_m.empty(); }

    explicit operator utf8_string() const {
        auto result = std::vector<uint8_t>();
        result.reserve(byte_count().v);
        for (const auto &e : data_m) {
            e.visit(
                [&](code_point_t cp) { cp.utf8_encode(result); },
                [&](const utf8_string &s) { meta::append(result, s); },
                [&](const utf8_view &v) { meta::append(result, v); });
        }
        return std::move(result);
    }

    bool operator==(const this_t &o) const {
        // TODO: avoid allocation
        auto tmp = static_cast<utf8_string>(*this);
        auto tmp2 = static_cast<utf8_string>(o);
        return tmp == tmp2;
    }
    bool operator!=(const this_t &o) const { return !(*this == o); }

    bool operator==(const utf8_view &v) const {
        // TODO: without allocation
        auto tmp = static_cast<utf8_string>(*this);
        return v.content_equals(utf8_view(tmp));
    }

private:
    std::vector<element_t> data_m;
};

inline utf8_string to_string(const rope &r) { return static_cast<utf8_string>(r); }

inline std::ostream &operator<<(std::ostream &out, const rope &r) { return out << to_string(r); }

} // namespace strings
