#pragma once
#include <scanner/NewLineIndentationValue.h>

#include <meta/Variant.ostream.h>

#include <ostream>

namespace meta {

constexpr auto nameOf(Type<scanner::MixedIndentCharacter>) { return "MixedIndentCharacter"; }

} // namespace meta

namespace scanner {

auto operator<<(std::ostream& out, const NewLineIndentErrors& errors) -> std::ostream&;

auto operator<<(std::ostream& out, const NewLineIndentationValue& lit) -> std::ostream&;

} // namespace scanner
