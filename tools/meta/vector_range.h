#pragma once

#include <type_traits>
#include <vector>

namespace meta {

template<class T>
struct vector_range {
    using this_t = vector_range;
    using vector_t = std::vector<std::remove_const_t<T>>;

    using value_type = typename vector_t::value_type;
    using allocator_type = typename vector_t::allocator_type;
    using pointer =
        std::conditional_t<std::is_const_v<T>, typename vector_t::const_pointer, typename vector_t::pointer>;
    using const_pointer = typename vector_t::const_pointer;
    using reference =
        std::conditional_t<std::is_const_v<T>, typename vector_t::const_reference, typename vector_t::reference>;
    using const_reference = typename vector_t::const_reference;
    using size_type = typename vector_t::size_type;
    using difference_type = typename vector_t::difference_type;
    using iterator =
        std::conditional_t<std::is_const_v<T>, typename vector_t::const_iterator, typename vector_t::iterator>;
    using const_iterator = typename vector_t::const_iterator;
    using reverse_iterator = std::conditional_t<
        std::is_const_v<T>,
        typename vector_t::const_reverse_iterator,
        typename vector_t::reverse_iterator>;
    using const_reverse_iterator = typename vector_t::const_reverse_iterator;

    vector_range(iterator b, iterator e)
        : b(b)
        , e(e) {}

    vector_range() = default;
    vector_range(const this_t &) = default;
    vector_range(this_t &&) = default;
    auto operator=(const this_t &) & -> this_t & = default;
    auto operator=(this_t &&) & -> this_t & = default;

    auto front() & -> reference { return *b; }
    auto front() const & -> const_reference { return *b; }
    auto back() & -> reference { return *(e - 1); }
    auto back() const & -> const_reference { return *(e - 1); }
    auto at(size_type pos) & -> reference {
        if (pos >= size()) throw std::out_of_range("index out of range");
        return *(b + pos);
    }
    auto at(size_type pos) const & -> const_reference { return const_cast<this_t *>(this)->at(pos); }

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

private:
    iterator b{};
    iterator e{};
};

} // namespace meta
