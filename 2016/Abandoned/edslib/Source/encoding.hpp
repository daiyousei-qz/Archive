/*=================================================================================
*  Copyright (c) 2016 Edward Cheng
*
*  edslib is an open-source library in C++ and licensed under the MIT License.
*  Refer to: https://opensource.org/licenses/MIT
*================================================================================*/

#pragma once
#include "string.hpp"
#include <stdexcept>
#include <algorithm>

namespace eds
{
    class BadEncodingError : std::runtime_error
    {
    public:
        BadEncodingError(const char *msg)
            : std::runtime_error(msg) { }
    };

    namespace detail
    {
        inline void EncodingAssert(bool condition, const char *msg = "")
        {
            if (!condition)
            {
                throw BadEncodingError(msg);
            }
        }
    }

    namespace ascii
    {
        inline char32_t ToLower(char32_t ch)
        {
            if (ch >= 'A' && ch <= 'Z')
            {
                return ch - 'A' + 'a';
            }
            else
            {
                return ch;
            }
        }
        inline char32_t ToUpper(char32_t ch)
        {
            if (ch >= 'a' && ch <= 'z')
            {
                return ch - 'a' + 'A';
            }
            else
            {
                return ch;
            }
        }
        inline bool IsAlphabet(char32_t ch)
        {
            return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
        }
        inline bool IsDigit(char32_t ch)
        {
            return ch >= '0' && ch <= '9';
        }

        inline bool Verify(char ch)
        {
            return (ch & 0x80) == 0;
        }
        inline bool Verify(StringView view)
        {
            auto pred = [](char ch) { return Verify(ch); };
            return std::all_of(view.begin(), view.end(), pred);
        }
    }

    namespace utf8
    {
        // note if leading_ch is invalid, 0 is returned
        inline size_t CodePointLength(char leading_ch)
        {
            static size_t lookup[] =
            {
                // 0x00~0x3f
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                // 0x40~0x7f
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                // 0x80~0xbf
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                // 0xc0~0xff
                2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,

            };

            auto index = std::char_traits<char>::to_int_type(leading_ch);
            return lookup[index];
        }

        inline bool IsLeadingByte(char ch)
        {
            return CodePointLength(ch) > 0;
        }

        inline bool IsContinuationByte(char ch)
        {
            return CodePointLength(ch) == 0;
        }

        inline size_t Decode(char32_t *output, const char *begin, const char *end)
        {
            Expects(output != nullptr);
            Expects(std::distance(begin, end) > 0);

            static constexpr const char *kErrorMsgUnexpectedChar = u8"utf8 unexpected character";
            static constexpr const char *kErrorMsgOutOfBound = u8"utf8 pointer overflows";
            static constexpr const char *kErrorMsgInvalidCodepoint = u8"utf8 invalid codepoint met";
            static constexpr const int kPrimaryMasks[] =
            {
                0b01111111, 0b00011111, 0b00001111, 0b00000111
            }; // keyed by (length - 1)
            static constexpr const char32_t kCodepointBoundaries[] =
            {
                0x7f, 0x07ff, 0xffff, 0x10ffff,
            }; // keyed by (length - 1)

            const char *p = begin;
            char ch = *(p++);
            detail::EncodingAssert(IsLeadingByte(ch), kErrorMsgUnexpectedChar);

            size_t len = CodePointLength(ch);
            char32_t result = ch & kPrimaryMasks[len - 1];
            for (size_t i = 1; i < len; ++i)
            {
                // test if p is within [begin, end)
                detail::EncodingAssert(p != end, kErrorMsgOutOfBound);

                ch = *(p++);
                // test if ch is utf8 continuation byte
                detail::EncodingAssert(IsContinuationByte(ch), kErrorMsgUnexpectedChar);

                result = (result << 6) | (ch & 0b00111111);
            }

            // unicode codepoint should be within 0 to 0x140000
            detail::EncodingAssert(result < kCodepointBoundaries[len - 1], kErrorMsgInvalidCodepoint);

            *output = result;
            return len;
        }

        inline bool Verify(StringView view)
        {
            const char *p = view.FrontPointer();
            size_t len = 0;
            while (p != view.BackPointer())
            {
                if (len == 0)
                {
                    if (IsLeadingByte(*p))
                    {
                        // calculate length of current codepoint
                        len = CodePointLength(*p);

                        // validate length 
                        switch (len)
                        {
                        case 2:
                            detail::EncodingAssert((*p & 0b00011110) == 0);
                            break;
                        case 3:
                        case 4:
                            detail::EncodingAssert((*p & 0b00001111) == 0);
                            break;
                        }
                    }
                    else
                    {
                        return false;
                    }
                }
                else if (!IsContinuationByte(*p))
                {
                    return false;
                }

                // update state
                ++p;
                --len;
            }

            return true;
        }

        inline std::u32string QuickDecode(const std::string &s)
        {
            const char *begin = s.c_str();
            const char *end = begin + s.size();
            size_t szRead = 0;

            char32_t buf;
            std::u32string result;
            while (szRead < s.size())
            {
                szRead += Decode(&buf, begin + szRead, end);
                result.push_back(buf);
            }

            detail::EncodingAssert(szRead == s.size(), "bad encoding");
            return result;
        }
    }

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