#pragma once
#include "CodePoint.h"
#include "String.h"

#include "meta/Optional.h"
#include "meta/algorithm.h"

#include <limits>
#include <string>

namespace strings {

struct View;
using OptionalView = meta::Optional<meta::DefaultPacked<View>>;

/// non owning read only view to a utf8 encoded string
struct View {
    using This = View;
    using Char = char;
    using Byte = uint8_t;
    static_assert(sizeof(Char) == sizeof(Byte), "char is not a byte");
    using It = const Char*;

private:
    It start_m{};
    It end_m{};

public:
    constexpr View() noexcept = default;
    constexpr View(const View&) = default;
    constexpr View(View&&) = default;
    constexpr View& operator=(const View&) = default;
    constexpr View& operator=(View&&) = default;

    template<size_t N>
    constexpr explicit View(const Char (&str)[N]) noexcept // view a constant string literal
        : View(It(str), It(&str[N - 1])) {}

    constexpr View(It start, It end) noexcept // from iterator range
        : start_m(start)
        , end_m(end) {}

    View(const String& s) noexcept // view existing owning string
        : View(s.begin(), s.end()) {}

    explicit View(const std::string& s) noexcept // view owning std::string
        : View(s.data(), s.data() + s.size()) {}

    // enable fast optional
    // equal if view on the same range
    // use isContentEqual for content comparison
    constexpr bool operator==(const This& o) const { return start_m == o.start_m && end_m == o.end_m; }
    constexpr bool operator!=(const This& o) const { return !(*this == o); }

    // byte ordering
    constexpr bool operator<(const This& o) const {
        auto l = *this;
        auto r = o;
        auto lcp = l.pop().orValue(0);
        auto rcp = r.pop().orValue(0);
        while (lcp != 0 && lcp == rcp) {
            lcp = l.pop().orValue(0);
            rcp = r.pop().orValue(0);
        }
        return (lcp < rcp);
    }

    constexpr bool isContentEqual(const This& o) const {
        return byteCount() == o.byteCount() && meta::equals(*this, o.begin());
    }
    constexpr bool isEmpty() const { return start_m == end_m; }
    constexpr bool isPartOf(const This& o) const { return begin() >= o.begin() && end() <= o.end(); }

    constexpr auto byteCount() const -> Counter { return {static_cast<uint32_t>(end_m - start_m)}; }
    constexpr auto size() const -> size_t { return static_cast<size_t>(end_m - start_m); }

    template<size_t N>
    constexpr auto skipBytes() const -> View {
        if (byteCount().v > N) return View{start_m + N, end_m};
        return View{end_m, end_m};
    }
    constexpr auto skipBytes(Counter n) const -> View {
        if (byteCount().v > n.v) return View{start_m + n.v, end_m};
        return View{end_m, end_m};
    }

    template<size_t N>
    constexpr auto firstBytes() const -> View {
        if (byteCount().v >= N) return View{start_m, start_m + N};
        return *this;
    }
    constexpr auto firstBytes(Counter n) const -> View {
        if (byteCount().v > n.v) return View{start_m, start_m + n.v};
        return *this;
    }

    constexpr auto data() const -> It { return start_m; }

    constexpr auto begin() const -> It { return start_m; }
    constexpr auto end() const -> It { return end_m; }

    constexpr auto front() const -> Byte { return static_cast<Byte>(*begin()); }

private:
    constexpr auto peek() -> meta::Optional<Byte> {
        if (0 == byteCount().v) return {};
        return front();
    }
    constexpr auto pop() -> meta::Optional<Byte> {
        if (0 == byteCount().v) return {};
        auto r = front();
        start_m++;
        return r;
    }
    template<size_t N>
    constexpr bool pop() {
        if (N > byteCount().v) {
            start_m = end_m;
            return false;
        }
        start_m += N;
        return true;
    }
};

struct CompareView : View {
    using View::View;
    CompareView() = default;
    CompareView(const View& v)
        : View(v.begin(), v.end()) {}
    constexpr bool operator==(const This& o) const { return View::operator==(o) || isContentEqual(o); }
    constexpr bool operator!=(const This& o) const { return !(*this == o); }
};

inline auto to_string(const View& v) -> String { return {v.begin(), v.end()}; }

} // namespace strings
