#include "Expression.h"

namespace parser {

bool NameTypeValueReference::operator==(const This& o) const {
    // note: during testing the parser generates the NameTypeValue and the reference to that,
    //   so we have no way to make sure that the pointers are equal to an expected tree.
    // return nameTypeValue == o.nameTypeValue;

    //   for now we make sure the name is equal - you have to ensure it is unique during a test!
    return nameTypeValue != nullptr && o.nameTypeValue != nullptr && nameTypeValue->name == o.nameTypeValue->name;
}

} // namespace parser
