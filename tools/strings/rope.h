#pragma once
#include "strings/code_point.h"
#include "strings/utf8_string.h"
#include "strings/utf8_view.h"

#include "meta/overloaded.h"
#include "meta/variant.h"

#include <vector>

namespace strings {

/// representation of a piecewise string
// use this to efficiently build large strings
struct rope {
    using element_t = meta::variant<code_point_t, utf8_string, utf8_view>;

    rope &operator+=(code_point_t c) {
        data_m.emplace_back(c);
        return *this;
    }
    rope &operator+=(utf8_string &&s) {
        data_m.emplace_back(std::move(s));
        return *this;
    }
    rope &operator+=(utf8_view v) {
        data_m.emplace_back(v);
        return *this;
    }

    count_t byte_count() const {
        return meta::accumulate(data_m, count_t{0}, [](count_t c, const element_t &e) {
            return meta::visit(meta::make_overloaded([=](code_point_t cp) { return c + cp.utf8_byte_count(); },
                                                     [=](const utf8_string &s) { return c + s.byte_count(); },
                                                     [=](const utf8_view &v) { return c + v.byte_count(); }),
                               e);
        });
    }

    explicit operator utf8_string() const {
        auto result = std::vector<uint8_t>();
        result.reserve(byte_count().v);
        for (const auto &e : data_m) {
            meta::visit(meta::make_overloaded([&](code_point_t cp) { cp.utf8_encode(result); },
                                              [&](const utf8_string &s) { meta::append(result, s); },
                                              [&](const utf8_view &v) { meta::append(result, v); }),
                        e);
        }
        return std::move(result);
    }

  private:
    std::vector<element_t> data_m;
};

} // namespace strings
