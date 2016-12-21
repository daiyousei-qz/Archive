#pragma once
#include "encoding.hpp"

namespace eds {
namespace regex {

    class Utf8Reader
    {
    public:
        Utf8Reader(const char *begin, const char *end)
            : begin_(begin), end_(end), cursor_(begin)
        {
            Expects(utf8::Verify(StringView{ begin, end }));
        }
        Utf8Reader(StringView view)
            : Utf8Reader(view.FrontPointer(), view.BackPointer()) { }

        bool Exhausted() const noexcept
        {
            return cursor_ == end_;
        }

        char32_t Peek() const
        {
            TestNotExhausted();
            EnsureCache();
            return cached_value_;
        }
        bool PeekIf(char ch) const
        {
            Expects(ascii::Verify(ch));

            return !Exhausted() && *cursor_ == ch;
        }
        bool PeekIf(StringView u8_prefix) const
        {
            Expects(utf8::Verify(u8_prefix));

            return !Exhausted() && StringView{ cursor_, end_ }.HasPrefix(u8_prefix);
        }

        char32_t Read()
        {
            char32_t result = Peek();
            MoveNext();

            return result;
        }
        bool ReadIf(char ch)
        {
            if (PeekIf(ch))
            {
                AdvanceCursor(1);
                return true;
            }
            else
            {
                return false;
            }
        }
        bool ReadIf(StringView u8_prefix)
        {
            if (PeekIf(u8_prefix))
            {
                AdvanceCursor(u8_prefix.Size());
                return true;
            }
            else
            {
                return false;
            }
        }

        void Seek(size_t position)
        {
            cursor_ = begin_ + position;
            Ensures(std::distance(cursor_, end_) >= 0);

            // to force a decode
            ClearCache();
            EnsureCache();
        }

        size_t Cursor() const noexcept
        {
            return std::distance(begin_, cursor_);
        }

        StringView BaseString() const noexcept
        {
            return StringView{ begin_, end_ };
        }
    private:
        void TestNotExhausted() const
        {
            if (Exhausted())
            {
                throw UnexpectedEOSError{};
            }
        }

        void ClearCache() const
        {
            cached_length_ = 0;
        }
        void EnsureCache() const
        {
            if (cached_length_ == 0)
            {
                cached_length_ = utf8::Decode(&cached_value_, cursor_, end_);
            }
        }

        void AdvanceCursor(size_t offset)
        {
            cursor_ += offset;
            ClearCache();
        }

        void MoveNext()
        {
            EnsureCache();
            AdvanceCursor(cached_length_);
        }
    private:
        // decoder cache
        mutable size_t cached_length_ = 0;
        mutable char32_t cached_value_ = 0;
        // underlying string reader
        const char *begin_;
        const char *end_;
        const char *cursor_;
    };
}
}