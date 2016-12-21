/*=================================================================================
*  Copyright (c) 2016 Edward Cheng
*
*  edslib is an open-source library in C++ and licensed under the MIT License.
*  Refer to: https://opensource.org/licenses/MIT
*================================================================================*/

#pragma once
#include "lhelper.hpp"
#include "type_utils.hpp"
#include <type_traits>
#include <algorithm>

namespace eds
{
    //=================================================================================
    // StringView

    template <typename TChar>
    class BasicStringView
    {
    public:
        using CharType = TChar;
        using PointerType = const TChar*;
        using IteratorType = const TChar*;
        using ConstIteratorType = const TChar*;

        using SupportedCharTypes = TypeList<char, wchar_t, char16_t, char32_t>;
        static_assert(TypeListOps::Contain<SupportedCharTypes, TChar>(), "");

    public:
        //
        // ctors
        //

        // constructed from string literal
        // note for array of N, the last byte is terminator 0
        template <size_t N>
        constexpr BasicStringView(const TChar(&str_literal)[N]) noexcept
            : begin_(str_literal), size_(N - 1) { }

        // constructed from interval given
        BasicStringView(const TChar *str, size_t sz) noexcept
            : begin_(str), size_(sz) { }

        // constructed from iterators
        BasicStringView(const TChar *begin, const TChar *end) noexcept
            : BasicStringView(begin, std::distance(begin, end)) { }

        // constructed from zstring
        explicit BasicStringView(const TChar *zstr)
            : BasicStringView(zstr, std::char_traits<TChar>::length(zstr)) { }

        // trivially copy constructible and assignable
        BasicStringView(const BasicStringView &) = default;
        BasicStringView &operator=(const BasicStringView &) = default;

        //
        // operations
        //

        const TChar *FrontPointer() const noexcept
        {
            return begin_;
        }

        const TChar *BackPointer() const noexcept
        {
            return begin_ + size_;
        }

        size_t Size() const noexcept
        {
            return size_;
        }

        bool IsEmpty() const noexcept
        {
            return size_ == 0;
        }

        bool HasPrefix(BasicStringView prefix) const
        {
            if (prefix.Size() > Size())
            {
                return false;
            }

            return std::equal(prefix.cbegin(), prefix.cend(), cbegin());
        }

        bool HasSuffix(BasicStringView suffix) const
        {
            if (suffix.Size() > Size())
            {
                return false;
            }

            return std::equal(suffix.cbegin(), suffix.cend(), cbegin() + Size() - suffix.Size());
        }

        BasicStringView RemovePrefix(size_t sz) const
        {
            Expects(sz < size_);
            return BasicStringView{ begin_ + sz, size_ - sz };
        }

        BasicStringView RemoveSuffix(size_t sz) const
        {
            Expects(sz < size_);
            return BasicStringView{ begin_, size_ - sz };
        }

        BasicStringView SubString(size_t offset, size_t sz) const
        {
            Expects(offset + sz < size_);
            return BasicStringView{ begin_ + offset, sz };
        }

        std::basic_string<TChar> ToString() const
        {
            return std::basic_string<TChar>{ begin_, size_ };
        }

        //
        // iterators
        //
        IteratorType begin() const { return cbegin(); }
        IteratorType end() const { return cend(); }

        ConstIteratorType cbegin() const { return begin_; }
        ConstIteratorType cend() const { return begin_ + size_; }

        //
        // swap
        //
        void swap(BasicStringView &&other)
        {
            std::swap(begin_, other.begin_);
            std::swap(size_, other.size_);
        }

        //
        // access
        //
        TChar At(size_t index) const
        {
            Expects(index < Size());
            return *(begin_ + index);
        }

        TChar operator[](size_t index) const
        {
            Expects(index < Size());
            return *(begin_ + index);
        }
    private:
        const TChar *begin_;
        const size_t size_;
    };

    template <typename TChar>
    bool operator==(const BasicStringView<TChar> &lhs, const BasicStringView<TChar> &rhs)
    {
        return lhs.FrontPointer() == rhs.FrontPointer()
            && lhs.Size() == rhs.Size();
    }

    template <typename TChar>
    bool operator!=(const BasicStringView<TChar> &lhs, const BasicStringView<TChar> &rhs)
    {
        return !operator==(lhs, rhs);
    }

    template <typename TChar>
    bool operator<(const BasicStringView<TChar> &lhs, const BasicStringView<TChar> &rhs)
    {
        return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
    }

    template <typename TChar>
    bool operator<=(const BasicStringView<TChar> &lhs, const BasicStringView<TChar> &rhs)
    {
        return operator==(lhs, rhs) || operator<(lhs, rhs);
    }

    template <typename TChar>
    bool operator>(const BasicStringView<TChar> &lhs, const BasicStringView<TChar> &rhs)
    {
        return !operator<=(lhs, rhs);
    }

    template <typename TChar>
    bool operator>=(const BasicStringView<TChar> &lhs, const BasicStringView<TChar> &rhs)
    {
        return !operator<(lhs, rhs);
    }

    using StringView = BasicStringView<char>;
    using WStringView = BasicStringView<wchar_t>;
    using U16StringView = BasicStringView<char16_t>;
    using U32StringView = BasicStringView<char32_t>;

    //=================================================================================
    // StringReader

    class UnexpectedEOSError : std::runtime_error
    {
    public:
        UnexpectedEOSError(const char *msg = "")
            : std::runtime_error(msg) { }
    };
    template <typename TChar>
    class BasicStringReader
    {
    public:
        using CharType = TChar;

    public:
        BasicStringReader(const TChar *begin, const TChar *end)
            : begin_(begin), end_(end), cursor_(begin)
        {
            Validate();
        }
        BasicStringReader(BasicStringView<TChar> view)
            : BasicStringReader(view.FrontPointer(), view.BackPointer()) { }

    public:
        //
        // states
        //
        bool Exhausted() const noexcept
        {
            return cursor_ == end_;
        }

        size_t RemainingCount() const noexcept
        {
            return std::distance(cursor_, end_);
        }

        size_t Size() const noexcept
        {
            return std::distance(begin_, end_);
        }

        BasicStringView<TChar> BaseString() const noexcept
        {
            return BasicStringView<TChar>{ begin_, end_ };
        }

        //
        // operations
        //
        TChar Peek() const
        {
            TestNotExhausted();
            return *cursor_;
        }
        bool PeekIf(TChar ch) const
        {
            return Peek() == ch;
        }
        bool PeekIf(BasicStringView<TChar> prefix) const
        {
            return BasicStringView<TChar>{ cursor_, end_ }.HasPrefix(prefix);
        }

        TChar Read()
        {
            TestNotExhausted();
            return *(cursor_++);
        }

        bool ReadIf(TChar ch)
        {
            bool result = PeekIf(ch);
            if (result)
            {
                cursor_ += 1;
            }

            return result;
        }

        bool ReadIf(BasicStringView<TChar> prefix)
        {
            bool result = PeekIf(prefix);
            if (result)
            {
                cursor_ += prefix.Size();
            }

            return result;
        }

        void Seek(int offset)
        {
            cursor_ += offset;
            Validate();
        }

        BasicStringReader Clone() const noexcept
        {
            return *this;
        }

    private:
        void TestNotExhausted() const noexcept
        {
            if (Exhausted())
            {
                throw UnexpectedEOSError{};
            }
        }

        void Validate()
        {
            Asserts(std::distance(begin_, cursor_) >= 0);
            Asserts(std::distance(cursor_, end_) >= 0);
        }

    private:
        const TChar *begin_;
        const TChar *end_;
        const TChar *cursor_;
    };

    using StringReader = BasicStringReader<char>;
    using WStringReader = BasicStringReader<wchar_t>;
    using U16StringReader = BasicStringReader<char16_t>;
    using U32StringReader = BasicStringReader<char32_t>;
}