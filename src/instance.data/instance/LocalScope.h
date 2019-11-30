#pragma once
#include "strings/View.h"

#include <map>

namespace instance {

using Name = strings::String;
using NameView = strings::View;

struct Entry;
using EntryView = Entry*;
using OptEntryView = meta::Optional<EntryView>;
using OptConstEntryView = meta::Optional<const Entry*>;

using EntryByName = std::multimap<NameView, Entry>;
template<class it>
struct Range {
    it _begin;
    it _end;

    bool empty() const { return _begin == _end; }
    bool single() const { return _begin != _end && std::next(_begin) == _end; }
    auto frontValue() const -> auto& { return _begin->second; }
    it begin() const { return _begin; }
    it end() const { return _end; }
};
using EntryRange = Range<EntryByName::iterator>;
using ConstEntryRange = Range<EntryByName::const_iterator>;

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
    LocalScope(This&&);
    auto operator=(This&&) -> This&;

    auto operator[](NameView name) const& noexcept -> ConstEntryRange;
    auto operator[](NameView name) & noexcept -> EntryRange;

    auto begin() noexcept -> EntryByName::iterator;
    auto end() noexcept -> EntryByName::iterator;

    auto emplace(Entry&& entry) & -> EntryView;

    // bool replace(old, new)
};

} // namespace instance
