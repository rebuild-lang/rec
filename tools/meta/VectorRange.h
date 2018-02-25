#pragma once

#include <type_traits>
#include <vector>

namespace meta {

template<class T>
struct VectorRange {
    using This = VectorRange;
    using VT = std::vector<std::remove_const_t<T>>;

    using value_type = typename VT::value_type;
    using allocator_type = typename VT::allocator_type;
    using pointer = std::conditional_t<std::is_const_v<T>, typename VT::const_pointer, typename VT::pointer>;
    using const_pointer = typename VT::const_pointer;
    using reference = std::conditional_t<std::is_const_v<T>, typename VT::const_reference, typename VT::reference>;
    using const_reference = typename VT::const_reference;
    using size_type = typename VT::size_type;
    using difference_type = typename VT::difference_type;
    using iterator = std::conditional_t<std::is_const_v<T>, typename VT::const_iterator, typename VT::iterator>;
    using const_iterator = typename VT::const_iterator;
    using reverse_iterator =
        std::conditional_t<std::is_const_v<T>, typename VT::const_reverse_iterator, typename VT::reverse_iterator>;
    using const_reverse_iterator = typename VT::const_reverse_iterator;

private:
    iterator b{};
    iterator e{};

public:
    VectorRange(iterator b, iterator e)
        : b(b)
        , e(e) {}

    VectorRange() = default;

    auto front() & -> reference { return *b; }
    auto front() const & -> const_reference { return *b; }
    auto back() & -> reference { return *(e - 1); }
    auto back() const & -> const_reference { return *(e - 1); }
    auto at(size_type pos) & -> reference {
        if (pos >= size()) throw std::out_of_range("index out of range");
        return *(b + pos);
    }
    auto at(size_type pos) const & -> const_reference { return const_cast<This *>(this)->at(pos); }

    auto operator[](size_type pos) & noexcept -> reference { return *(b + pos); }
    auto operator[](size_type pos) const & noexcept -> const_reference { return *(b + pos); }

    auto begin() & noexcept -> iterator { return b; }
    auto begin() const & noexcept -> const_iterator { return b; }
    auto cbegin() const & noexcept -> const_iterator { return b; }

    auto end() & noexcept -> iterator { return e; }
    auto end() const & noexcept -> const_iterator { return e; }
    auto cend() const & noexcept -> const_iterator { return e; }

    auto size() const & noexcept -> size_type { return e - b; }
    bool empty() const &noexcept { return b == e; }
};

} // namespace meta
