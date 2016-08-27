/*=================================================================================
*  Copyright (c) 2016 Edward Cheng
*
*  edslib is an open-source library in C++ and licensed under the MIT License.
*  Refer to: https://opensource.org/licenses/MIT
*================================================================================*/

#pragma once
#include "type_checker.hpp"
#include <type_traits>
#include <iterator>

namespace eds
{
    //=====================================================================================
    // is_iterator

    // test via if category_tag could be found
    template <typename T, typename = void>
    struct is_iterator : public std::false_type { };
    template <typename T>
    struct is_iterator<T, std::void_t<typename std::iterator_traits<T>::iterator_category>>
        : public std::true_type { };

    //=====================================================================================
    // TypeList
    template <typename ...Ts>
    struct TypeList { };

    namespace detail
    {
        //
        // IsTypeListHelper
        //
        template <typename TList>
        struct IsTypeListHelper : std::false_type { };
        template <typename ...Ts>
        struct IsTypeListHelper<TypeList<Ts...>> : std::true_type { };

        //
        // TypeListSizeHelper
        //
        template <typename TList>
        struct TypeListSizeHelper;
        template <typename ...Ts>
        struct TypeListSizeHelper<TypeList<Ts...>>
            : std::integral_constant<size_t, sizeof...(Ts)> { };

        //
        // TypeListContainHelper
        //
        template <typename T, typename TList>
        struct TypeListContainHelper;
        template <typename T, typename ...Ts>
        struct TypeListContainHelper<T, TypeList<Ts...>>
            : std::bool_constant<
                std::disjunction<std::is_same<T, Ts>...>::value> { };

        //
        // TypeListIndexOfHelper
        //
        template <size_t Offset, typename T, typename TList>
        struct TypeListIndexOfHelper;
        template <size_t Offset, typename T, typename First, typename ...Ts>
        struct TypeListIndexOfHelper<Offset, T, TypeList<First, Ts...>>
            : std::integral_constant<
                size_t,
                std::is_same<T, First>::value
                    ? Offset
                    : TypeListIndexOfHelper<Offset + 1, T, TypeList<Ts...>>::value> { };
        template <size_t Offset, typename T>
        struct TypeListIndexOfHelper<Offset, T, TypeList<>>
            : std::integral_constant<size_t, -1> { };

        //
        // TypeListRemovePrefixHelper
        //
        template <typename TList>
        struct TypeListNextListHelper;
        template <typename T, typename ...Ts>
        struct TypeListNextListHelper<TypeList<T, Ts...>>
        {
            using Type = TypeList<Ts...>;
        };
        template <>
        struct TypeListNextListHelper<TypeList<>> { };

        //
        // TypeListAtHelper
        //
        template <long long Index, typename TList>
        struct TypeListAtHelper;
        template <long long Index, typename T, typename ...Ts>
        struct TypeListAtHelper<Index, TypeList<T, Ts...>>
        {
            using Type = std::conditional_t<
                Index == 0,
                T,
                typename TypeListAtHelper<Index - 1, TypeList<Ts...>>::Type>;
        };
        template <long long Index, typename T>
        struct TypeListAtHelper<Index, TypeList<T>>
        {
            // workaround: some unused instantions may generate negative Index
            static_assert(Index <= 0, "Index out of bound");
            using Type = T;
        };
    }

    class TypeListOps
    {
    public:
        template <typename TList>
        static constexpr bool IsTypeList()
        {
            return detail::IsTypeListHelper<TList>::value;
        }

        template <typename TList>
        static constexpr bool IsEmpty()
        {
            return detail::TypeListSizeHelper<TList>::value == 0;
        }

        template <typename TList>
        static constexpr size_t Size()
        {
            return detail::TypeListSizeHelper<TList>::value;
        }

        template <typename TList, typename T>
        static constexpr bool Contain()
        {
            return detail::TypeListContainHelper<T, TList>::value;
        }

        template <typename TList, typename T>
        static constexpr size_t IndexOf()
        {
            static_assert(Contain<TList, T>(), "type T not in the TypeList");
            return detail::TypeListIndexOfHelper<0, T, TList>::value;
        }

    public:
        template <typename TList>
        using NextList = typename detail::TypeListNextListHelper<TList>::Type;

        template <typename TList, size_t Index>
        using TypeAt = typename detail::TypeListAtHelper<Index, TList>::Type;
        template <typename TList>
        using FrontType = TypeAt<TList, 0>;
        template <typename TList>
        using BackType = TypeAt<TList, Size<TList>() - 1>;
    };
}