#include "LocalScope.h"

#include "Entry.h"

namespace instance {

LocalScope::LocalScope() = default;
LocalScope::~LocalScope() = default;

LocalScope::LocalScope(This&&) noexcept = default;
auto LocalScope::operator=(This&&) noexcept -> LocalScope& = default;

auto LocalScope::byName(NameView name) const& noexcept -> ConstEntryRange { return m.equalRange(name); }
auto LocalScope::updateRange(NameView name) & noexcept -> EntryRange { return m.updateRange(name); }

auto LocalScope::begin() noexcept -> EntryByName::It { return m.begin(); }
auto LocalScope::end() noexcept -> EntryByName::It { return m.end(); }
auto LocalScope::begin() const noexcept -> EntryByName::cIt { return m.begin(); }
auto LocalScope::end() const noexcept -> EntryByName::cIt { return m.end(); }

auto LocalScope::emplace(Entry&& entry) & -> void { m.insert(std::move(entry)); }

} // namespace instance
