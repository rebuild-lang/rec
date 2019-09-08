#include "LocalScope.h"

#include "Entry.h"

namespace instance {

LocalScope::LocalScope() { new (&m_storage) EntryByName(); }

LocalScope::~LocalScope() { m().~EntryByName(); }

LocalScope::LocalScope(This&& o) { new (&m_storage) EntryByName(std::move(o.m())); }

auto LocalScope::operator=(This&& o) -> This& {
    new (&m_storage) EntryByName(std::move(o.m()));
    return *this;
}

auto LocalScope::operator[](NameView name) const& -> ConstEntryRange {
    auto [b, e] = m().equal_range(name);
    return {b, e};
}

auto LocalScope::operator[](NameView name) & -> EntryRange {
    auto [b, e] = m().equal_range(name);
    return {b, e};
}

auto LocalScope::begin() -> EntryByName::iterator { return m().begin(); }
auto LocalScope::end() -> EntryByName::iterator { return m().end(); }

inline auto LocalScope::m() -> EntryByName& { return *std::launder(reinterpret_cast<EntryByName*>(&m_storage)); }
inline auto LocalScope::m() const -> const EntryByName& {
    return *std::launder(reinterpret_cast<const EntryByName*>(&m_storage));
}

auto LocalScope::emplaceImpl(Entry&& entry) & -> EntryView {
    auto name = nameOf(entry);
    return &m().emplace(name, std::move(entry))->second;
}

} // namespace instance
