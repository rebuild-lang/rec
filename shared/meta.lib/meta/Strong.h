#pragma once
#include "Type.h"
#include "TypePack.h"

#include <type_traits>

namespace meta {

template<class Value, class... Tags>
struct Strong {
    static_assert(isUnique<Tags...>(), "Tags have to be unique");
    Value v{};

    constexpr Strong() = default;
    constexpr explicit Strong(Value v)
        : v(v) {}
};

template<class Value, class... InnerTags, class... Tags>
struct Strong<Strong<Value, InnerTags...>, Tags...> {
    static_assert(std::is_same_v<Strong<Value, InnerTags...>, void>, "Do not nest Strong types!");
};

template<class Value, class... Tags>
constexpr auto makeStrong(TypePack<Tags...> = {}, Type<Value> = {}) -> Type<Strong<Value, Tags...>> {
    return {};
}

/// extract value type
template<class Value>
constexpr auto toValueType(Type<Value>) -> Type<Value> {
    return {};
}
template<class Value, class... Tags>
constexpr auto toValueType(Type<Strong<Value, Tags...>>) -> Type<Value> {
    return {};
}
template<class T>
using ToValueType = decltype(toValueType(Type<T>{}));

template<class T>
using ToBareValueType = ToBareType<ToValueType<T>>;

/// extract value
template<class Value>
constexpr auto toValue(Value value) {
    return value;
}
template<class Value, class... Tags>
constexpr auto toValue(Strong<Value, Tags...> s) {
    return s.v;
}

/// tag inspection
template<class CandidateTag, class Value, class... Tags>
constexpr auto hasTag(Type<Strong<Value, Tags...>>, Type<CandidateTag> = {}) {
    return contains<CandidateTag>(TypePack<Tags...>{});
}

/// tag manipulations
template<class NewTag, class Value, class... Tags>
constexpr auto addTag(Type<Strong<Value, Tags...>>, Type<NewTag> = {}) {
    return Type<Strong<Value, Tags..., NewTag>>{};
}
template<class Strong, class NewTag>
using StrongAddTag = decltype(addTag(Type<Strong>{}, Type<NewTag>{}));

template<class... NewTags, class Value, class... Tags>
constexpr auto addTags(Type<Strong<Value, Tags...>>, TypePack<NewTags...> = {}) {
    return Type<Strong<Value, Tags..., NewTags...>>{};
}
template<class Strong, class... NewTags>
using AddTags = decltype(addTags(Type<Strong>{}, TypePack<NewTags...>{}));

template<class Tag, class Value, class... Tags>
constexpr auto removeTag(Type<Strong<Value, Tags...>>, Type<Tag> = {}) {
    constexpr auto tagsPack = TypePack<Tags...>{};
    static_assert(!contains<Tag>(tagsPack), "Removed Tag is not present!");
    return makeStrong<Value>(remove<Tag>(tagsPack));
}

template<class OriginalTag, class NewTag, class Value, class... Tags>
constexpr auto replaceTag(Type<Strong<Value, Tags...>>, Type<OriginalTag> = {}, Type<NewTag> = {}) {
    constexpr auto tagsPack = TypePack<Tags...>{};
    static_assert(!contains<OriginalTag>(tagsPack), "Removed Tag is not present!");
    static_assert(contains<NewTag>(tagsPack), "Added Tag is already present!");

    return Type<Strong<Value, std::conditional_t<Type<Tags>{} == Type<OriginalTag>{}, NewTag, Tags>...>>{};
}

template<class NewValue, class Value, class... Tags>
constexpr auto replaceValue(Type<Strong<Value, Tags...>>, Type<NewValue> = {}) //
    -> Type<Strong<NewValue, Tags...>> {
    return {};
}

/// Comparison and Ordering
#define BOOL_OP(OP)                                                                                                    \
    template<class T, class... Tags>                                                                                   \
    constexpr bool operator OP(Strong<T, Tags...> a, Strong<T, Tags...> b) {                                           \
        return a.v OP b.v;                                                                                             \
    }

BOOL_OP(==)
BOOL_OP(!=)
BOOL_OP(<)
BOOL_OP(>)
BOOL_OP(<=)
BOOL_OP(>=)

#undef BOOL_OP

} // namespace meta

namespace std {

template<class T, class... Tags>
struct hash<meta::Strong<T, Tags...>> {
    std::size_t operator()(meta::Strong<T, Tags...> a) const { return a.v; }
};

} // namespace std
