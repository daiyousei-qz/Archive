/*=================================================================================
*  Copyright (c) 2016 Edward Cheng
*
*  edslib is an open-source library in C++ and licensed under the MIT License.
*  Refer to: https://opensource.org/licenses/MIT
*================================================================================*/

#pragma once
#include "lhelper.hpp"
#include "type_utils.hpp"
#include <cstddef>
#include <iterator>
#include <algorithm>
#include <type_traits>

namespace eds
{
    // small is a locally contained dynamic sequence container with limited capacity
    // if overflowed, exception should be thrown
    // also, iterators invalidate when swap() is called
    template <typename T, size_t N = 16>
    class static_vector
    {
        // NOTE
        // 1. don't modify count_ before using iterators

    public:
        // no allocator_type as no allocation is needed
        using value_type                = T;
        using size_type                 = std::size_t;
        using difference_type           = std::ptrdiff_t;
        using reference                 = value_type&;
        using const_reference           = const value_type&;
        using pointer                   = T*;
        using const_pointer             = const T*;

        using iterator                  = pointer;
        using const_iterator            = const_pointer;
        using reverse_iterator          = std::reverse_iterator<iterator>;
        using const_reverse_iterator    = std::reverse_iterator<const_iterator>;

    public:
        // ctor
        static_vector() { }
        template <typename InputIt, 
                  typename = std::enable_if_t<is_iterator<InputIt>::value>>
        static_vector(InputIt first, InputIt last)
        {
            assign(first, last);
        }
        static_vector(const static_vector& other)
        {
            assign(other.begin(), other.end());
        }
        static_vector(static_vector&& other)
        {
            swap(std::forward<static_vector>(other));
        }
        static_vector(std::initializer_list<T> ilist)
        {
            assign(ilist);
        }

        static_vector& operator=(const static_vector& other)
        {
            assign(other.begin(), other.end());
            return *this;
        }
        static_vector& operator=(static_vector&& other)
        {
            clear();
            swap(other);
            return *this;
        }

        // dtor
        ~static_vector()
        {
            clear();
        }

    public:
        // assign
        void assign(size_type count, const T& value)
        {
            for (size_type i = 0; i < count; ++i)
            {
                emplace_back(value);
            }
        }

        template <typename InputIt,
                  typename = std::enable_if_t<is_iterator<InputIt>::value>>
        void assign(InputIt first, InputIt last)
        {
            clear();
            for (InputIt p = first; p != last; ++p)
            {
                emplace_back(*p);
            }
        }

        void assign(std::initializer_list<T> ilist)
        {
            assign(ilist.begin(), ilist.end());
        }

        // element access
        reference at(size_type pos)
        {
            _CheckIndex(pos);
            return data()[pos];
        }
        const_reference at(size_type pos) const
        {
            _CheckIndex(pos);
            return data()[pos];
        }
        reference operator[](size_type pos)
        {
            _CheckIndex(pos);
            return data()[pos];
        }
        const_reference operator[](size_type pos) const
        {
            _CheckIndex(pos);
            return data()[pos];
        }

        reference front()
        {
            _CheckNonEmpty();
            return *begin();
        }
        const_reference front() const
        {
            _CheckNonEmpty();
            return *begin();
        }
        reference back()
        {
            _CheckNonEmpty();
            return *rbegin();
        }
        const_reference back() const
        {
            _CheckNonEmpty();
            return *rbegin();
        }
        T* data() noexcept
        {
            return reinterpret_cast<T*>(&data_[0]);
        }
        const T* data() const noexcept
        {
            return reinterpret_cast<const T*>(&data_[0]);
        }

        // iterator
        iterator       begin()           noexcept { return data(); }
        iterator       end()             noexcept { return data() + size(); }
        const_iterator begin()     const noexcept { return data(); }
        const_iterator end()       const noexcept { return data() + size(); }
        const_iterator cbegin()    const noexcept { return data(); }
        const_iterator cend()      const noexcept { return data() + size(); }

        reverse_iterator rbegin() noexcept
        {
            return reverse_iterator{ end() };
        }
        reverse_iterator rend() noexcept
        {
            return reverse_iterator{ begin() };
        }
        const_reverse_iterator rbegin() const noexcept
        {
            return const_reverse_iterator{ cend() };
        }
        const_reverse_iterator rend() const noexcept
        {
            return const_reverse_iterator{ cbegin() };
        }
        const_reverse_iterator crbegin() const noexcept
        {
            return const_reverse_iterator{ cend() };
        }
        const_reverse_iterator crend() const noexcept
        {
            return const_reverse_iterator{ cbegin() };
        }

        // capacity
        bool empty() const noexcept
        {
            return count_ == 0;
        }
        size_type size() const noexcept
        {
            return count_;
        }
        constexpr size_type max_size() const noexcept
        {
            return N;
        }
        constexpr size_type capacity() const noexcept
        {
            return N;
        }

        // modifier
        void clear()
        {
            while(!empty())
            { 
                pop_back();
            }
        }
        
        template <typename ... TArgs>
        iterator emplace(const_iterator pos, TArgs&& ... args)
        {
            _TestInternalIterator(pos);
            
            iterator mutable_pos = const_cast<iterator>(pos);
            _InsertSlot(mutable_pos, 1);
            _EmplaceAt(mutable_pos, std::forward<TArgs>(args)...);

            return mutable_pos;
        }

        iterator insert(const_iterator pos, const T& value)
        {
            return emplace(pos, value);
        }
        iterator insert(const_iterator pos, T&& value)
        {
            return emplace(pos, std::forward<T>(value));
        }
        iterator insert(const_iterator pos, size_type count, const T& value)
        {
            _TestInternalIterator(pos);

            iterator mutable_pos = const_cast<iterator>(pos);
            _InsertSlot(mutable_pos, count);
            std::fill_n(mutable_pos, count, value);
            
            return mutable_pos;
        }
        template <typename InputIt,
                  typename = std::enable_if_t<is_iterator<InputIt>::value>>
        iterator insert(const_iterator pos, InputIt first, InputIt last)
        {
            _TestInternalIterator(pos);

            iterator mutable_pos = const_cast<iterator>(pos);
            size_t sz = std::distance(first, last);
            _InsertSlot(mutable_pos, sz);
            std::copy(first, last, mutable_pos);

            return mutable_pos;
        }
        iterator insert(const_iterator pos, std::initializer_list<T> ilist)
        {
            return insert(pos, ilist.begin(), ilist.end());
        }

        iterator erase(const_iterator pos)
        {
            return erase(pos, std::next(pos));
        }
        iterator erase(const_iterator first, const_iterator last)
        {
            _TestInternalIterator(first, last);

            size_type counter = 0; // how many elements erased
            for (auto p = first; p != last; ++p)
            {
                counter += 1;
                _DisposeAt(const_cast<iterator>(p));
            }

            std::move(const_cast<iterator>(last), end(), const_cast<iterator>(first));
            count_ -= counter;
            return const_cast<iterator>(first);
        }

        void push_back(const T& value)
        {
            emplace_back(value);
        }
        void push_back(T&& value)
        {
            emplace_back(std::forward<T>(value));
        }
        template <typename ... TArgs>
        void emplace_back(TArgs&& ... args)
        {
            _EnsureCapacity(1);
            _EmplaceAt(&data()[count_++], std::forward<TArgs>(args)...);
        }
        void pop_back()
        {
            _DisposeAt(&data()[--count_]);
        }

        void swap(static_vector &other)
        {
            if (size() > other.size())
            {
                other.swap(*this);
            }
            else
            {
                // swap contents
                iterator rest_begin = std::swap_ranges(begin(), end(), other.begin());

                auto src = rest_begin;
                auto dest = end();
                while (src != other.end())
                {
                    new (dest) T(std::move(*src));
                    ++src;
                    ++dest;
                }

                // swap counter
                std::swap(count_, other.count_);
            }
        }
    private:
        //
        // precondition checker
        //
        void _EnsureCapacity(size_t requested)
        {
            Asserts(size() + requested <= max_size());
        }
        template <typename TIter>
        void _TestInternalIterator(TIter where) const
        {
            Asserts(std::distance(begin(), where) >= 0);
            Asserts(std::distance(where, end()) >= 0);
        }
        template <typename TIter>
        void _TestInternalIterator(TIter first, TIter last) const
        {
            Asserts(std::distance(begin(), first) >= 0);
            Asserts(std::distance(first, last) >= 0);
            Asserts(std::distance(last, end()) >= 0);
        }
        void _CheckNonEmpty() const
        {
            Asserts(!empty());
        }
        void _CheckIndex(size_type index) const
        {
            Asserts(index >= 0 && index < size());
        }
        //
        // construct an element at $where with args
        //
        template <typename ... TArgs>
        void _EmplaceAt(iterator where, TArgs&& ... args)
        {
            // security warning, maybe unsafe
            new (where) T{ std::forward<TArgs>(args)... };
        }
        //
        // dispose the element at $where
        //
        void _DisposeAt(iterator where)
        {
            // security warning, maybe unsafe
            where->~T();
        }
        //
        // reserves $n slots before elements at $where
        //
        void _InsertSlot(iterator where, size_type n)
        {
            _EnsureCapacity(n);

            std::move_backward(where, end(), end() + n);
            count_ += n;
        }

    private:
        // count of elements in the instance
        using slot_type = std::aligned_storage_t<sizeof(T), alignof(T)>;

        slot_type data_[N];
        size_type count_ = 0;
    };

    //
    // operator overload
    //
    template <typename T, size_t N>
    inline bool operator==(const static_vector<T, N>& lhs, const static_vector<T, N>& rhs)
    {
        return lhs.size() == rhs.size() &&
                std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
    }

    template <typename T, size_t N>
    inline bool operator!=(const static_vector<T, N>& lhs, const static_vector<T, N>& rhs)
    {
        return !(lhs == rhs);
    }

    template <typename T, size_t N>
    inline bool operator<(const static_vector<T, N>& lhs, const static_vector<T, N>& rhs)
    {
        return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
    }

    template <typename T, size_t N>
    inline bool operator<=(const static_vector<T, N>& lhs, const static_vector<T, N>& rhs)
    {
        return (lhs < rhs || lhs == rhs);
    }

    template <typename T, size_t N>
    inline bool operator>(const static_vector<T, N>& lhs, const static_vector<T, N>& rhs)
    {
        return !(lhs < rhs || lhs == rhs);
    }

    template <typename T, size_t N>
    inline bool operator>=(const static_vector<T, N>& lhs, const static_vector<T, N>& rhs)
    {
        return !(lhs < rhs);
    }
}