#pragma once
#include "text_range.h"
#include <deque>

namespace scanner {

using char_t = strings::code_point_t;
using opt_char_t = strings::optional_code_point_t;

struct file_input_t {
    using value_ptr = const string_t::value_type *;

    file_input_t(const file_t &file)
        : file_ptr(&file)
        , begin_ptr(file.content.data())
        , current_ptr(file.content.data())
        , peek_ptr(file.content.data())
        , begin_position({})
        , current_position({}) {}

    bool has_more_peek() const { return peek_ptr != end_ptr(); }

    auto range() const {
        return text_range{file_ptr, view_t(begin_ptr, current_ptr), begin_position, current_position};
    }

    void collapse() {
        begin_ptr = current_ptr;
        begin_position = current_position;
    }

    void rollback() {
        current_ptr = begin_ptr;
        current_position = begin_position;
        peek_ptr = current_ptr;
        peek_buffer.clear();
    }

    template<size_t index = 0>
    opt_char_t peek_char() {
        if (peek_buffer.size() <= index) {
            if (!fill_peek(index + 1)) return {};
        }
        return peek_buffer[index];
    }

    bool skip();

    bool extend(column_t tabstop = {});

    bool extend(size_t count) {
        for (auto i = 0u; i < count; ++i)
            if (!extend()) return false;
        return true;
    }

    void extend_white_spaces(column_t tabstop) {
        while (peek_char().map([=](char_t chr) {
            if (!chr.is_white_space()) return false;
            extend(tabstop);
            return true;
        }))
            ;
    }

    void collapse_white_spaces(column_t tabstop) {
        extend_white_spaces(tabstop);
        collapse();
    }

    void next_line() { current_position.next_line(); }

    value_ptr current() const { return current_ptr; }

private:
    value_ptr end_ptr() const { return file_ptr->content.data() + file_ptr->content.byte_count().v; }
    bool fill_peek(size_t count);

private:
    const file_t *file_ptr;
    value_ptr begin_ptr;
    value_ptr current_ptr;
    value_ptr peek_ptr;
    position_t begin_position;
    position_t current_position;
    std::deque<char_t> peek_buffer;
};

} // namespace scanner
