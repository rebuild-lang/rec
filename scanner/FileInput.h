#pragma once
#include "TextRange.h"
#include <deque>

namespace scanner {

using CodePoint = strings::CodePoint;
using OptCodePoint = strings::OptionalCodePoint;

struct FileInput {
    using StringIterator = const String::Data*;

    FileInput(const File& file)
        : file(&file)
        , begin_(file.content.data())
        , current_(file.content.data())
        , peek_(file.content.data())
        , beginPosition({})
        , currentPosition({}) {}

    bool hasMorePeek() const { return peek_ != end_(); }
    bool hasMoreBytes(strings::Count bytes) const { return current_ + bytes.v <= end_(); }

    auto view() const { return View(begin_, current_); }
    auto range() const { return TextRange{file, view(), beginPosition, currentPosition}; }

    void collapse() {
        begin_ = current_;
        beginPosition = currentPosition;
    }

    void rollback() {
        current_ = begin_;
        currentPosition = beginPosition;
        peek_ = current_;
        peekBuffer.clear();
    }

    template<size_t index = 0>
    auto peek() -> OptCodePoint {
        if (peekBuffer.size() <= index) {
            if (!fillPeek(index + 1)) return {};
        }
        return peekBuffer[index];
    }

    auto currentView(strings::Count bytes) -> View { return View(current_, current_ + bytes.v); }

    bool skip();

    bool extend(Column tabstop = {});

    bool extend(size_t count) {
        for (auto i = 0u; i < count; ++i)
            if (!extend()) return false;
        return true;
    }

    void extendWhiteSpaces(Column tabstop) {
        while (peek().map([=](CodePoint chr) {
            if (!chr.isWhiteSpace()) return false;
            extend(tabstop);
            return true;
        })) {
            // noop
        }
    }

    void collapseWhiteSpaces(Column tabstop) {
        extendWhiteSpaces(tabstop);
        collapse();
    }

    void nextLine() { currentPosition.nextLine(); }

    auto current() const -> StringIterator { return current_; }

    void finish() { current_ = end_(); }

private:
    auto end_() const -> StringIterator { return file->content.data() + file->content.byteCount().v; }
    bool fillPeek(size_t count);

private:
    const File* file{};
    StringIterator begin_{};
    StringIterator current_{};
    StringIterator peek_{};
    Position beginPosition{};
    Position currentPosition{};
    std::deque<CodePoint> peekBuffer{};
};

} // namespace scanner
