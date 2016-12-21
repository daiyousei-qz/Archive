/*=================================================================================
*  Copyright (c) 2016 Edward Cheng
*
*  edslib is an open-source library in C++ and licensed under the MIT License.
*  Refer to: https://opensource.org/licenses/MIT
*================================================================================*/

#pragma once
#include "type_utils.hpp"
#include <cstddef>
#include <vector>
#include <algorithm>

namespace eds
{
    template <typename Key,
        typename T,
        typename Compare = std::less<Key>,
        typename Allocator = std::allocator<Key>>
        class compact_map
    {
        static_assert(std::is_move_constructible<Key>::value, "!!!");
        static_assert(std::is_move_constructible<T>::value, "!!!");

    public:
        using key_type                  = Key;
        using mapped_type               = T;
        using value_type                = std::pair<const Key, T>;
        using size_type                 = std::size_t;
        using difference_type           = std::ptrdiff_t;
        using key_compare               = Compare;
        using allocator_type            = Allocator;
        using reference                 = value_type&;
        using const_reference           = const value_type&;
        using pointer                   = typename std::allocator_traits<Allocator>::pointer;
        using const_pointer             = typename std::allocator_traits<Allocator>::const_pointer;
 
        using underlying_container      = std::vector<value_type>;
        using iterator                  = underlying_container::iterator;
        using const_iterator            = underlying_container::const_iterator;
        using reverse_iterator          = underlying_container::reverse_iterator;
        using const_reverse_iterator    = underlying_container::const_reverse_iterator;

    public:
        // ctor
        compact_set() { }
        template <typename InputIt>
        compact_set(InputIt first, InputIt last)
        {
            assign(first, last);
        }
        compact_set(const compact_set& other)
        {
            assign(other.begin(), other.end());
        }
        compact_set(compact_set&& other)
        {
            swap(other);
        }
        compact_set(std::initializer_list<Key> ilist)
        {
            assign(ilist);
        }

        compact_set& operator=(const compact_set& other)
        {
            assign(other.begin(), other.end());
            return *this;
        }
        compact_set& operator=(compact_set&& other)
        {
            clear();
            swap(other);
            return *this;
        }

        //
        // dtor
        //
        ~compact_set() = default;

    public:
        //
        // assign
        //
        template <typename InputIt>
        std::enable_if_t<is_iterator<InputIt>::value>
            assign(InputIt first, InputIt last)
        {
            clear();
            insert(first, last);
        }

        void assign(std::initializer_list<Key> ilist)
        {
            assign(ilist.begin(), ilist.end());
        }

        //
        // access
        //
        reference at(size_type pos)
        {
            return _container[pos];
        }
        const_reference at(size_type pos) const
        {
            return _container[pos];
        }
        reference operator[](size_type pos)
        {
            return _container[pos];
        }
        const_reference operator[](size_type pos) const
        {
            return _container[pos];
        }

        //
        // iterator
        //
        iterator        begin()        noexcept { return _container.begin(); }
        iterator        end()          noexcept { return _container.end(); }
        const_iterator  begin()  const noexcept { return _container.cbegin(); }
        const_iterator  end()    const noexcept { return _container.cend(); }
        const_iterator  cbegin() const noexcept { return _container.cbegin(); }
        const_iterator  cend()   const noexcept { return _container.cend(); }

        reverse_iterator       rbegin()        noexcept { return _container.begin(); }
        reverse_iterator       rend()          noexcept { return _container.end(); }
        const_reverse_iterator rbegin()  const noexcept { return _container.cbegin(); }
        const_reverse_iterator rend()    const noexcept { return _container.cend(); }
        const_reverse_iterator rcbegin() const noexcept { return _container.cbegin(); }
        const_reverse_iterator rcend()   const noexcept { return _container.cend(); }

        //
        // capacity
        //
        bool empty() const noexcept
        {
            return _container.empty();
        }
        size_type capacity() const noexcept
        {
            return _container.capacity();
        }
        size_type size() const noexcept
        {
            return _container.size();
        }
        size_type max_size() const noexcept
        {
            return _container.max_size();
        }

        //
        // modifiers
        //
        void clear()
        {
            _container.clear();
        }

        template <typename ... TArgs>
        std::pair<iterator, bool> emplace(TArgs&& ... args)
        {
            // TODO: performance warning
            Key value = Key(std::forward<TArgs>(args)...);
            iterator lb = lower_bound(value);
            if (lb != _container.end() && *lb == value)
            {
                return std::pair<iterator, bool>{lb, false};
            }
            else
            {
                iterator where = _container.emplace(lb, std::forward<TArgs>(args)...);
                return std::pair<iterator, bool>{where, true};
            }
        }
        std::pair<iterator, bool> insert(const value_type& value)
        {
            return emplace(value);
        }
        std::pair<iterator, bool> insert(value_type&& value)
        {
            return emplace(std::forward<value_type>(value));
        }
        iterator insert(const_iterator hint, const value_type& value)
        {
            // hint is ignored
            return emplace(value);
        }
        iterator insert(const_iterator hint, value_type&& value)
        {
            // hint is ignored
            return emplace(std::forward<value_type>(value));
        }
        template <typename InputIt>
        void insert(InputIt first, InputIt last)
        {
            _container.insert(_container.end(), first, last);
            std::sort(_container.begin(), _container.end(), key_comp());
        }
        void insert(std::initializer_list<value_type> ilist)
        {
            insert(ilist.begin(), ilist.end());
        }

        void erase(const Key &x)
        {
            auto where = find(x);
            if (where != _container.end())
            {
                _container.erase(where);
            }
        }

        void swap(compact_set& other)
        {
            _container.swap(other._container);
        }

        // lookup
        size_type count(const Key& value) const
        {
            return std::find(_container.begin(), _container.end(), value) == end() ? 0 : 1;
        }
        iterator find(const Key& value)
        {
            return std::find(_container.begin(), _container.end(), value);
        }
        const_iterator find(const Key& value) const
        {
            return std::find(_container.cbegin(), _container.cend(), value);
        }
        iterator lower_bound(const Key& value)
        {
            return std::lower_bound(begin(), end(), value, key_comp());
        }
        const_iterator lower_bound(const Key& value) const
        {
            return std::lower_bound(begin(), end(), value, key_comp());
        }
        iterator upper_bound(const Key& value)
        {
            return std::upper_bound(begin(), end(), value, key_comp());
        }
        const_iterator upper_bound(const Key& value) const
        {
            return std::upper_bound(begin(), end(), value, key_comp());
        }

        // observers
        key_compare   key_comp()   const { return key_compare{}; }
        value_compare value_comp() const { return value_compare{}; }

    private:
        std::vector<Key, Allocator> _container;
    };

    template <typename Key,
        typename Compare = std::less<Key>,
        typename Allocator = std::allocator<Key>>
        inline bool operator ==(const compact_set<Key, Compare, Allocator> &lhs,
            const compact_set<Key, Compare, Allocator> &rhs)
    {
        return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
    }

    template <typename Key,
        typename Compare = std::less<Key>,
        typename Allocator = std::allocator<Key>>
        inline bool operator !=(const compact_set<Key, Compare, Allocator> &lhs,
            const compact_set<Key, Compare, Allocator> &rhs)
    {
        return !(lhs == rhs);
    }

    template <typename Key,
        typename Compare = std::less<Key>,
        typename Allocator = std::allocator<Key>>
        inline bool operator <(const compact_set<Key, Compare, Allocator> &lhs,
            const compact_set<Key, Compare, Allocator> &rhs)
    {
        return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(), Compare{});
    }

    template <typename Key,
        typename Compare = std::less<Key>,
        typename Allocator = std::allocator<Key>>
        inline bool operator <=(const compact_set<Key, Compare, Allocator> &lhs,
            const compact_set<Key, Compare, Allocator> &rhs)
    {
        return operator==(lhs, rhs) || operator<(lhs, rhs);
    }

    template <typename Key,
        typename Compare = std::less<Key>,
        typename Allocator = std::allocator<Key>>
        inline bool operator >(const compact_set<Key, Compare, Allocator> &lhs,
            const compact_set<Key, Compare, Allocator> &rhs)
    {
        return !operator<=(lhs, rhs);
    }

    template <typename Key,
        typename Compare = std::less<Key>,
        typename Allocator = std::allocator<Key>>
        inline bool operator >=(const compact_set<Key, Compare, Allocator> &lhs,
            const compact_set<Key, Compare, Allocator> &rhs)
    {
        return !operator<(lhs, rhs);
    }
}