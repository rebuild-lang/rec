#pragma once
#include "File.h"
#include "Range.h"

#include <deque>

namespace text {

using CodePoint = strings::CodePoint;
using OptCodePoint = strings::OptionalCodePoint;

struct FileInput {
    using StringIterator = const String::Char*;

    explicit FileInput(const File& file)
        : file(&file)
        , begin_(file.content.data())
        , current_(file.content.data())
        , peek_(file.content.data())
        , beginPosition({})
        , currentPosition_({}) {}

    bool hasMore() const { return current_ != end_(); }
    bool hasMoreBytes(strings::Counter bytes) const { return end_() >= current_ && size_t(end_() - current_) <= bytes.v; }

    auto view() const { return View(begin_, current_); }
    auto range() const { return Range{file, view(), beginPosition, currentPosition_}; }

    void collapse() {
        begin_ = current_;
        beginPosition = currentPosition_;
    }

    void rollback() {
        current_ = begin_;
        currentPosition_ = beginPosition;
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

    auto currentView(strings::Counter bytes) -> View { return View(current_, current_ + bytes.v); }

    bool skip();

    bool extend(Column tabstop = {});

    bool extend(size_t count) {
        for (auto i = 0u; i < count; ++i) {
            if (!extend()) return false;
        }
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

    void nextLine() { currentPosition_.nextLine(); }

    auto current() const -> StringIterator { return current_; }
    auto currentPosition() const -> Position { return currentPosition_; }

    void restoreCurrent(StringIterator it, Position position) {
        current_ = it;
        currentPosition_ = position;
    }

    void finish() { current_ = end_(); }

private:
    auto end_() const -> StringIterator { return file->content.end(); }
    bool fillPeek(size_t count);

private:
    const File* file{};
    StringIterator begin_{};
    StringIterator current_{};
    StringIterator peek_{};
    Position beginPosition{};
    Position currentPosition_{};
    std::deque<CodePoint> peekBuffer{};
};

} // namespace text
