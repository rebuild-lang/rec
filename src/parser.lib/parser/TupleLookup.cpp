#include "TupleLookup.h"

namespace parser {

auto LocalTupleLookup::operator[](View name) const& -> OptNameTypeValueView {
    for (auto& ntv : tuple->tuple) {
        if (ntv.name && name.isContentEqual(ntv.name.value())) return &ntv;
    }
    return {};
}

} // namespace parser
