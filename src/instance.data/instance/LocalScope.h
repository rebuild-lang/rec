#pragma once
#include "Type.h"

#include "meta/Same.h"
#include "meta/Variant.h"
#include "strings/View.h"

#include <algorithm>
#include <vector>

namespace instance {

using Name = strings::String;
using NameView = strings::View;
using meta::Variant;

template<class it>
struct Range {
    Range() = default;
    Range(it b, it e) noexcept
        : _begin(b)
        , _end(e) {}

    [[nodiscard]] auto empty() const noexcept -> bool { return _begin == _end; }
    [[nodiscard]] auto single() const noexcept -> bool { return _begin != _end && std::next(_begin) == _end; }
    [[nodiscard]] auto frontValue() const noexcept -> auto& { return *_begin; }
    [[nodiscard]] auto begin() const noexcept -> it { return _begin; }
    [[nodiscard]] auto end() const noexcept -> it { return _end; }

private:
    it _begin{};
    it _end{};
};
template<class T, class LessPred = std::less<T>>
struct OrderedVector {
    using Vec = std::vector<T>;
    using It = typename Vec::iterator;
    using cIt = typename Vec::const_iterator;

    template<class K>
    [[nodiscard]] auto equalRange(const K& k) const noexcept -> Range<cIt> {
        auto [b, e] = std::equal_range(vec.begin(), vec.end(), k, LessPred{});
        return Range<cIt>{b, e};
    }
    template<class K>
    [[nodiscard]] auto updateRange(const K& k) noexcept -> Range<It> {
        auto [b, e] = std::equal_range(vec.begin(), vec.end(), k, LessPred{});
        return Range<It>{b, e};
    }

    [[nodiscard]] auto count() const noexcept -> uint64_t { return vec.size(); }
    [[nodiscard]] auto capacity() const noexcept -> uint64_t { return vec.capacity(); }

    [[nodiscard]] auto begin() const noexcept -> cIt { return vec.begin(); }
    [[nodiscard]] auto end() const noexcept -> cIt { return vec.end(); }
    [[nodiscard]] auto begin() noexcept -> It { return vec.begin(); }
    [[nodiscard]] auto end() noexcept -> It { return vec.end(); }

    auto insert(T&& v) & -> It {
        auto it = std::upper_bound(vec.begin(), vec.end(), v, LessPred{});
        return vec.insert(it, std::move(v));
    }

    void reserve(uint64_t capacity) & { vec.reserve(capacity); }

private:
    Vec vec;
};

struct Entry;
using EntryView = Entry*;

inline auto nameOf(const NameView& view) -> NameView { return view; }

template<class T>
auto nameOf(const T& v) -> std::enable_if_t<meta::same<Name, decltype(v.name)>, NameView> {
    return v.name;
}
template<class T>
auto nameOf(const std::shared_ptr<T>& ptr) -> NameView {
    return nameOf(*ptr);
}
template<class... Ts>
auto nameOf(const meta::Variant<Ts...>& var) -> NameView {
    return var.visit([](const auto& v) -> NameView { return nameOf(v); });
}

struct EntryLess {
    using is_transparent = void;

    template<class A, class B>
    auto operator()(const A& a, const B& b) -> bool {
        return nameOf(a) < nameOf(b);
    }
};

using EntryByName = OrderedVector<Entry, EntryLess>;
using EntryRange = Range<EntryByName::It>;
using ConstEntryRange = Range<EntryByName::cIt>;

struct LocalScope {
    using This = LocalScope;

private:
    EntryByName m; // note map is not fully known here, so we implement all life cycle methods

public:
    LocalScope();
    ~LocalScope();

    // non copyable
    LocalScope(const This&) = delete;
    auto operator=(const This&) -> This& = delete;
    // move enabled
    LocalScope(This&&) noexcept;
    auto operator=(This&&) noexcept -> LocalScope&;

    [[nodiscard]] auto byName(NameView name) const& noexcept -> ConstEntryRange;
    [[nodiscard]] auto updateRange(NameView name) & noexcept -> EntryRange;

    [[nodiscard]] auto begin() noexcept -> EntryByName::It;
    [[nodiscard]] auto end() noexcept -> EntryByName::It;
    [[nodiscard]] auto begin() const noexcept -> EntryByName::cIt;
    [[nodiscard]] auto end() const noexcept -> EntryByName::cIt;

    auto emplace(Entry&& entry) & -> void;

    // bool replace(old, new)
};
using LocalScopePtr = std::shared_ptr<LocalScope>;
using ConstLocalScopePtr = std::shared_ptr<const LocalScope>;

} // namespace instance
