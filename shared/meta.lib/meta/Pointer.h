#pragma once

namespace meta {

template<class T>
static constexpr T* nullptr_to = nullptr;

template<auto*>
struct Ptr {};

template<auto* P>
static constexpr Ptr<P>* ptr_to = nullptr;

} // namespace meta
