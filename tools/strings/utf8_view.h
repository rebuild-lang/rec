#pragma once
#include "strings/code_point.h"
#include "strings/utf8_string.h"

#include "meta/algorithm.h"
#include "meta/optional.h"

#include <string>

namespace strings {

struct utf8_view;
using optional_utf8_view = meta::optional<meta::packed<utf8_view>>;

/// non owning read only view to a utf8 encoded string
struct utf8_view {
    using value_type = uint8_t;

    constexpr utf8_view() noexcept // valid empty view
        : utf8_view(nullptr, nullptr) {}

    template<size_t N>
    constexpr explicit utf8_view(const char (&str)[N]) noexcept // view a constant string literal
        : utf8_view(reinterpret_cast<const value_type *>(str), reinterpret_cast<const value_type *>(str + N - 1)) {}

    constexpr utf8_view(const value_type *start, const value_type *end) noexcept // from iterator range
        : start_m(start)
        , end_m(end) {}

    explicit utf8_view(const utf8_string &s) noexcept // view existing owning string
        : utf8_view(s.data(), s.data() + s.byte_count().v) {}

    explicit utf8_view(const std::string &s) noexcept // view owning std::string
        : utf8_view(reinterpret_cast<const value_type *>(s.data()),
                    reinterpret_cast<const value_type *>(s.data() + s.length())) {}

    // full value semantics
    constexpr utf8_view(const utf8_view &) noexcept = default;
    constexpr utf8_view &operator=(const utf8_view &) noexcept = default;
    constexpr utf8_view(utf8_view &&) noexcept = default;
    constexpr utf8_view &operator=(utf8_view &&) noexcept = default;

    // enable fast optional
    // equal if view on the same range
    // use content_equals for content comparison
    constexpr bool operator==(const utf8_view &o) const { return start_m == o.start_m && end_m == o.end_m; }

    constexpr bool content_equals(const utf8_view &o) const {
        return byte_count().v == o.byte_count().v && meta::equals(*this, o.begin());
    }

    constexpr count_t byte_count() const { return {static_cast<uint32_t>(end_m - start_m)}; }

    /// view of the first N bytes
    template<size_t N>
    constexpr optional_utf8_view front() const {
        if (byte_count().v >= N) {
            return utf8_view(start_m, start_m + N);
        }
        return {};
    }

    bool pull_bom();

    optional_code_point_t pull_code_point() {
        // see https://en.wikipedia.org/wiki/UTF-8
        return pop().map([=](value_type c0) -> optional_code_point_t {
            if ((c0 & 0x80) != 0x80) return code_point_t{c0};
            if ((c0 & 0xE0) == 0xC0) {
                return pop().map([=](value_type c1) -> optional_code_point_t {
                    if ((c1 & 0xC0) != 0x80) return {};
                    return code_point_t{((c0 & 0x1Fu) << 6) | ((c1 & 0x3Fu) << 0)};
                });
            }
            if ((c0 & 0xF0) == 0xE0) {
                return pop().map([=](value_type c1) -> optional_code_point_t {
                    if ((c1 & 0xC0) != 0x80) return {};
                    return pop().map([=](value_type c2) -> optional_code_point_t {
                        if ((c2 & 0xC0) != 0x80) return {};
                        return code_point_t{((c0 & 0x0Fu) << 12) | ((c1 & 0x3Fu) << 6) | ((c2 & 0x3Fu) << 0)};
                    });
                });
            }
            if ((c0 & 0xF8) == 0xF0) {
                return pop().map([=](value_type c1) -> optional_code_point_t {
                    if ((c1 & 0xC0) != 0x80) return {};
                    return pop().map([=](value_type c2) -> optional_code_point_t {
                        if ((c2 & 0xC0) != 0x80) return {};
                        return pop().map([=](value_type c3) -> optional_code_point_t {
                            if ((c3 & 0xC0) != 0x80) return {};
                            return code_point_t{((c0 & 0x07u) << 18) | ((c1 & 0x3Fu) << 12) | ((c2 & 0x3Fu) << 6) |
                                                ((c3 & 0x3Fu) << 0)};
                        });
                    });
                });
            }
            return {};
        });
    }

    constexpr const value_type *begin() const { return start_m; }
    constexpr const value_type *end() const { return end_m; }

private:
    meta::optional<value_type> peek() {
        if (0 == byte_count().v) return {};
        return *start_m;
    }
    meta::optional<value_type> pop() {
        if (0 == byte_count().v) return {};
        auto v = *start_m;
        start_m++;
        return v;
    }
    template<size_t N>
    bool pop() {
        if (N > byte_count().v) {
            start_m = end_m;
            return false;
        }
        start_m += N;
        return true;
    }

private:
    const value_type *start_m;
    const value_type *end_m;
};

inline bool utf8_view::pull_bom() {
    return front<3>().map([=](utf8_view v) {
        if (v.content_equals(utf8_view("\xEF\xBB\xBF"))) {
            pop<3>();
            return true;
        }
        return false;
    });
}

} // namespace strings
