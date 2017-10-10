#include "tokenizer.h"

bool scanner::file_input_t::skip() {
    if (peek_buffer.size() == 1) {
        peek_buffer.clear();
        current_ptr = peek_ptr;
        return true;
    }
    auto view = view_t(current_ptr, end_ptr());
    return view.pull_code_point().map([&](auto) {
        if (!peek_buffer.empty()) peek_buffer.pop_front();
        current_ptr = view.begin();
        return true;
    });
}

bool scanner::file_input_t::extend(scanner::column_t tabstop) {
    auto view = view_t(current_ptr, end_ptr());
    return view.pull_code_point().map([&](auto chr) {
        if (chr == '\t')
            current_position.next_tabstop(tabstop);
        else
            current_position.next_column();
        if (!peek_buffer.empty()) peek_buffer.pop_front();
        current_ptr = view.begin();
        return true;
    });
}

bool scanner::file_input_t::fill_peek(size_t count) {
    while (peek_buffer.size() < count) {
        auto view = view_t(peek_ptr, end_ptr());
        auto chr = view.pull_code_point();
        peek_ptr = view.begin();
        if (!chr) return false;
        peek_buffer.push_back(chr.value());
    }
    return true;
}
