#pragma once
#include "CodePoint.h"
#include "String.h"
#include "View.h"

#include "meta/Overloaded.h"
#include "meta/Variant.h"
#include "meta/algorithm.h"

#include <vector>

namespace strings {

/// representation of a piecewise string
// use this to efficiently build large strings
struct Rope {
    using This = Rope;
    using Char = char;
    using Data = meta::Variant<CodePoint, String, View>;

private:
    std::vector<Data> m{};

public:
    Rope() = default; // valid empty rope

    explicit Rope(const View& v) { m.emplace_back(v); }

    // append operators
    This& operator+=(CodePoint c) {
        m.emplace_back(c);
        return *this;
    }
    This& operator+=(String&& s) {
        if (!s.isEmpty()) m.emplace_back(std::move(s));
        return *this;
    }
    This& operator+=(View v) {
        if (v.isEmpty()) return *this;
        if (!m.empty() && m.back().holds<View>()) {
            auto& mv = m.back().get<View>();
            if (mv.end() == v.begin()) {
                mv = View(mv.begin(), v.end());
                return *this;
            }
        }
        m.emplace_back(v);
        return *this;
    }
    This& operator+=(Rope& r) {
        m.insert(m.end(), r.m.begin(), r.m.end());
        return *this;
    }

    Counter byteCount() const {
        return meta::accumulate(m, Counter{0}, [](Counter c, const Data& e) {
            return e.visit(
                [=](CodePoint cp) { return c + cp.utf8_byteCount(); },
                [=](const String& s) { return c + s.byteCount(); },
                [=](const View& v) { return c + v.byteCount(); });
        });
    }
    bool isEmpty() const { return m.empty(); }

    explicit operator String() const {
        auto result = std::vector<Char>();
        result.reserve(byteCount().v);
        for (const auto& e : m) {
            e.visit(
                [&](CodePoint cp) { cp.utf8_encode(result); },
                [&](const String& s) { meta::append(result, s); },
                [&](const View& v) { meta::append(result, v); });
        }
        return String{std::move(result)};
    }

    bool operator==(const This& o) const {
        // TODO(arBmind): eliminate temporary allocations (when used outside of tests)
        auto tmp = static_cast<String>(*this);
        auto tmp2 = static_cast<String>(o);
        return tmp == tmp2;
    }
    bool operator!=(const This& o) const { return !(*this == o); }

    bool operator==(const View& v) const {
        // TODO(arBmind): eliminate temporary allocation (when used outside of tests)
        auto tmp = static_cast<String>(*this);
        return v.isContentEqual(View(tmp));
    }
    bool operator!=(const View& o) const { return !(*this == o); }
};

inline String to_string(const Rope& r) { return static_cast<String>(r); }

} // namespace strings
