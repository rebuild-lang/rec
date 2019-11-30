#include "LocalScope.h"

#include "Entry.h"

namespace instance {

LocalScope::LocalScope() = default;
LocalScope::~LocalScope() = default;

LocalScope::LocalScope(This&&) = default;
auto LocalScope::operator=(This&&) -> This& = default;

auto LocalScope::operator[](NameView name) const& noexcept -> ConstEntryRange {
    auto [b, e] = m.equal_range(name);
    return {b, e};
}

auto LocalScope::operator[](NameView name) & noexcept -> EntryRange {
    auto [b, e] = m.equal_range(name);
    return {b, e};
}

auto LocalScope::begin() noexcept -> EntryByName::iterator { return m.begin(); }
auto LocalScope::end() noexcept -> EntryByName::iterator { return m.end(); }

auto LocalScope::emplace(Entry&& entry) & -> EntryView {
    auto name = nameOf(entry);
    return &m.emplace(name, std::move(entry))->second;
}

} // namespace instance
