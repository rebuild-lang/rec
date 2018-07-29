#pragma once

namespace meta {

namespace details {

void reportUnreachable();

} // namespace details

template<class Result = void>
auto unreachable() -> Result {
    details::reportUnreachable();
    return {};
}

} // namespace meta
