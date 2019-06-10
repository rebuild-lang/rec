#include "NewLineIndentationValue.ostream.h"

#include <strings/join.ostream.h>
#include <text/DecodedPosition.ostream.h>

namespace scanner {

auto operator<<(std::ostream& out, const NewLineIndentErrors& errors) -> std::ostream& {
    for (const auto& error : errors) {
        out << " error: " << error << '\n';
    }
    return out;
}

auto operator<<(std::ostream& out, const NewLineIndentationValue& lit) -> std::ostream& {
    out << "\\n-> " << lit.indentColumn;
    if (!lit.errors.empty()) out << "; errors: " << strings::joinEach(lit.errors, ", ");
    return out;
}

} // namespace scanner
