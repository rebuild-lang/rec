#pragma once
#include "strings/CodePoint.h"
#include "strings/String.h"
#include "strings/View.h"

#include "meta/Overloaded.h"
#include "meta/Variant.h"
#include "meta/algorithm.h"

#include <vector>

namespace strings {

/// representation of a piecewise string
// use this to efficiently build large strings
struct Rope {
    using This = Rope;
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
        if (!v.isEmpty()) m.emplace_back(v);
        return *this;
    }

    Count byteCount() const {
        return meta::accumulate(m, Count{0}, [](Count c, const Data& e) {
            return e.visit(
                [=](CodePoint cp) { return c + cp.utf8_byteCount(); },
                [=](const String& s) { return c + s.byteCount(); },
                [=](const View& v) { return c + v.byteCount(); });
        });
    }
    bool isEmpty() const { return m.empty(); }

    explicit operator String() const {
        auto result = std::vector<uint8_t>();
        result.reserve(byteCount().v);
        for (const auto& e : m) {
            e.visit(
                [&](CodePoint cp) { cp.utf8_encode(result); },
                [&](const String& s) { meta::append(result, s); },
                [&](const View& v) { meta::append(result, v); });
        }
        return std::move(result);
    }

    bool operator==(const This& o) const {
        // TODO: avoid allocation
        auto tmp = static_cast<String>(*this);
        auto tmp2 = static_cast<String>(o);
        return tmp == tmp2;
    }
    bool operator!=(const This& o) const { return !(*this == o); }

    bool operator==(const View& v) const {
        // TODO: without allocation
        auto tmp = static_cast<String>(*this);
        return v.isContentEqual(View(tmp));
    }
};

inline String to_string(const Rope& r) { return static_cast<String>(r); }

} // namespace strings
