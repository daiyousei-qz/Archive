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
        inline size_t CodePointLength(char leading_octet)
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
                4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0,
            };

            int index = std::char_traits<char>::to_int_type(leading_octet);
            return lookup[index];
        }

        inline bool IsLeadingByte(char ch)
        {
            return CodePointLength(ch) > 0;
        }

        inline bool IsContinuationByte(char ch)
        {
            return (ch & 0b11000000) == 0b10000000;
        }

        inline size_t Decode(char32_t *output, const char *begin, const char *end)
        {
            Expects(output != nullptr);
            Expects(std::distance(begin, end) > 0);

            static constexpr const char *kErrorMsgUnexpectedChar = u8"utf8 unexpected byte";
            static constexpr const char *kErrorMsgOutOfBound = u8"utf8 pointer overflows";
            static constexpr const char *kErrorMsgInvalidCodepoint = u8"utf8 invalid codepoint met";
            static constexpr const int kPrimaryMasks[] =
            {
                0b01111111, 0b00011111, 0b00001111, 0b00000111
            }; // keyed by (length - 1)
            static constexpr const char32_t kCodepointBoundaries[] =
            {
                0x80, 0x0800, 0x010000, 0x110000,
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
            static constexpr const int kOverLongTestMasks[] =
            {
                0b01111111, 0b00011110, 0b00001111, 0b00000111
            }; // keyed by (length - 1)

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

                        // validate not over long
                        if (*p & kOverLongTestMasks[len - 1] == 0 && p != 0)
                        {
                            return false;
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
}