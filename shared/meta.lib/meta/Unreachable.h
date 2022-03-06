#pragma once

namespace meta {

namespace details {

[[noreturn]] void reportUnreachable();

} // namespace details

template<class Result = void>
[[noreturn]] auto unreachable() -> Result {
    details::reportUnreachable();
    // return {};
}

} // namespace meta
