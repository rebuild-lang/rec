#include "FileInput.h"

namespace text {

bool FileInput::skip() {
    if (peekBuffer.size() == 1) {
        peekBuffer.clear();
        current_ = peek_;
        return true;
    }
    auto view = View(current_, end_());
    return view.pullCodePoint().map([&](auto) {
        if (!peekBuffer.empty()) peekBuffer.pop_front();
        current_ = view.begin();
        return true;
    });
}

bool FileInput::extend(Column tabstop) {
    auto view = View(current_, end_());
    return view.pullCodePoint().map([&](auto chr) {
        if (chr == '\t')
            currentPosition_.nextTabstop(tabstop);
        else
            currentPosition_.nextColumn();
        if (!peekBuffer.empty()) peekBuffer.pop_front();
        current_ = view.begin();
        return true;
    });
}

bool FileInput::fillPeek(size_t count) {
    while (peekBuffer.size() < count) {
        auto view = View(peek_, end_());
        auto chr = view.pullCodePoint();
        peek_ = view.begin();
        if (!chr) return false;
        peekBuffer.push_back(chr.value());
    }
    return true;
}

} // namespace text
