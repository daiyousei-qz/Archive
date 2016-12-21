#pragma once
#include "lhelper.hpp"
#include "std_utility.hpp"
#include "type_utils.hpp"
#include <memory>

namespace eds
{
    template <size_t I>
    using Index = std::integral_constant<size_t, I>;

    template <typename ...Ts>
    union ConstexprVariantUnion { };

    template <typename T, typename ...Ts>
    union ConstexprVariantUnion<T, Ts...>
    {
        // ctors
        // recurse down and match the node when I becomes 0
        template <typename ...TArgs>
        explicit constexpr ConstexprVariantUnion(Index<0>, TArgs&& ...args)
            : head_(std::forward<TArgs>(args)...) { }
        template <size_t I, typename ...TArgs>
        explicit constexpr ConstexprVariantUnion(Index<I>, TArgs&& ...args)
            : tail_(Index<I - 1>{}, std::forward<TArgs>(args)...) { }

        // dtor
        ~ConstexprVariantUnion() = default;

        // operations
        T &Extract(Index<0>)
        {
            return head_;
        }
        constexpr const T &Extract(Index<0>) const
        {
            return head_;
        }
        template <size_t I>
        TypeListOps::TypeAt<TypeList<T, Ts...>, I>
            &Extract(Index<I>)
        {
            return tail_.Extract(Index<I - 1>{});
        }
        template <size_t I>
        const TypeListOps::TypeAt<TypeList<T, Ts...>, I>
            &Extract(Index<I>) const
        {
            return tail_.Extract(Index<I - 1>{});
        }

    private:
        T head_;
        ConstexprVariantUnion<Ts...> tail_;
    };

    template <typename ...Ts>
    struct ConstexprVariantBase
    {
    public:
        template <size_t I, typename ...TArgs>
        explicit constexpr ConstexprVariantBase(Index<I>, TArgs&& ...args)
            : type_index_(I)
            , buffer_(Index<I>{}, std::forward<TArgs>(args)...)
        { }

        ~ConstexprVariantBase() = default;

        // operations
        size_t TypeIndex() const noexcept
        {
            return type_index_;
        }

        template <size_t I>
        TypeListOps::TypeAt<TypeList<Ts...>, I>
            &Extract(Index<I>) noexcept
        {
            return buffer_.Extract(Index<I>{});
        }
        template <size_t I>
        constexpr const TypeListOps::TypeAt<TypeList<Ts...>, I>
            &Extract(Index<I>) const noexcept
        {
            return buffer_.Extract(Index<I>{});
        }

        template <size_t I, typename T, typename ...TArgs>
        void Initialize(TArgs&& ...args)
        {

        }

    private:
        size_t type_index_;
        ConstexprVariantUnion<Ts...> buffer_;
    };

    template <typename ...Ts>
    union RuntimeVariantUnion { };

    template <typename T, typename ...Ts>
    union RuntimeVariantUnion<T, Ts...>
    {
        // ctors
        // recurse down and match the node when I becomes 0
        template <typename ...TArgs>
        explicit constexpr RuntimeVariantUnion(Index<0>, TArgs&& ...args)
            : head_(std::forward<TArgs>(args)...) { }
        template <size_t I, typename ...TArgs>
        explicit constexpr RuntimeVariantUnion(Index<I>, TArgs&& ...args)
            : tail_(Index<I - 1>{}, std::forward<TArgs>(args)...) { }

        // dtor
        ~RuntimeVariantUnion() { /* do nothing */ }

        // operations
        size_t TypeIndex() const noexcept
        {
            return type_index_;
        }

        T &Extract(Index<0>)
        {
            return head_;
        }
        constexpr const T &Extract(Index<0>) const
        {
            return head_;
        }
        template <size_t I>
        TypeListOps::TypeAt<TypeList<T, Ts...>, I>
            &Extract(Index<I>)
        {
            return tail_.Extract(Index<I - 1>{});
        }
        template <size_t I>
        const TypeListOps::TypeAt<TypeList<T, Ts...>, I>
            &Extract(Index<I>) const
        {
            return tail_.Extract(Index<I - 1>{});
        }

    private:
        T head_;
        RuntimeVariantUnion<Ts...> tail_;
    };

    template <typename ...Ts>
    struct RuntimeVariantBase
    {
    public:
        template <size_t I, typename ...TArgs>
        RuntimeVariantBase(Index<I>, TArgs&& ...args)
            : type_index_(I)
            , buffer_(Index<I>{}, std::forward<TArgs>(args)...)
        { }

        ~RuntimeVariantBase()
        {
            using DestroyerPtr = void(*)(void*);
            static constexpr DestroyerPtr destroyers[] = { &DestroyAt<Ts>... };

            Asserts(type_index_ < sizeof...(Ts));
            destroyers[type_index_](std::addressof(buffer_));

#pragma warning()
            // possible optimization:
            // set destroyer of trivial destructible types to nullptr
            // to eliminate a redundant function call
        }

        template <typename T>
        static void DestroyAt(void *ptr)
        {
            reinterpret_cast<T*>(ptr)->~T();
        }
    public:
        size_t TypeIndex() const noexcept
        {
            return type_index_;
        }

        template <size_t I>
        TypeListOps::TypeAt<TypeList<Ts...>, I>
            &Extract(Index<I>) noexcept
        {
            return buffer_.Extract(Index<I>{});
        }
        template <size_t I>
        constexpr const TypeListOps::TypeAt<TypeList<Ts...>, I>
            &Extract(Index<I>) const noexcept
        {
            return buffer_.Extract(Index<I>{});
        }

        template <size_t I, typename ...TArgs>
        void Initialize(TArgs&& ...args)
        {

        }

    private:
        size_t type_index_;
        RuntimeVariantUnion<Ts...> buffer_;
    };

    template <typename ...Ts>
    using VariantBase = std::conditional_t<
        std::conjunction<std::is_trivially_destructible<Ts>...>::value,
        ConstexprVariantBase<Ts...>,
        RuntimeVariantBase<Ts...>>;

    template <typename T>
    struct VariantTypeWrapperHelper
    {
        using Type = T;
    };
    template <typename T>
    struct VariantTypeWrapperHelper<T&>
    {
        using Type = std::reference_wrapper<T>;
    };
    template <typename T>
    struct VariantTypeWrapperHelper<const T&>
    {
        using Type = const std::reference_wrapper<T>;
    };

    template <typename T>
    using VariantWrappedType = typename VariantTypeWrapperHelper<T>::Type;

    //
    // Helpers to find best match in Ts... for a type U
    //
    template <typename TList, size_t Offset>
    struct EmptyInvoker;
    template <size_t Offset, typename T, typename ...Ts>
    struct EmptyInvoker<TypeList<T, Ts...>, Offset>
        : public EmptyInvoker<TypeList<Ts...>, Offset + 1>
    {
        //static_assert(!TypeListOps::Contain<TypeList<Ts...>, T>(), "overloads cannot be found");

        using FuncPtr = Index<Offset>(*)(T);
        operator FuncPtr() { return nullptr; }
    };
    template <size_t Offset>
    struct EmptyInvoker<TypeList<>, Offset> { };

    template <typename TList, typename U>
    using BestMatchIndex = decltype(std::declval<EmptyInvoker<TList, 0>>()(std::declval<U>()));

    //=================================================================================

    static constexpr size_t variant_npos = -1;

    //
    // bad_variant_access
    //
    class bad_variant_access : public std::exception
    {
    public:
        bad_variant_access() noexcept = default;
    };

    //
    // monostate
    //
    struct monostate { };
    constexpr bool operator<(monostate, monostate) noexcept { return false; }
    constexpr bool operator>(monostate, monostate) noexcept { return false; }
    constexpr bool operator<=(monostate, monostate) noexcept { return true; }
    constexpr bool operator>=(monostate, monostate) noexcept { return true; }
    constexpr bool operator==(monostate, monostate) noexcept { return true; }
    constexpr bool operator!=(monostate, monostate) noexcept { return false; }

    /* std::hash<monostate> specialization */

    //
    // variant
    //
    template <typename ...Ts>
    class variant : private VariantBase<VariantWrappedType<Ts>...>
    {
    public:
        using BaseType = VariantBase<VariantWrappedType<Ts>...>;
        using OriginalTypeList = TypeList<Ts...>;
        using WrappedTypeList = TypeList<VariantWrappedType<Ts>...>;

        template <typename TOriginal>
        using OriginalIndexOf = Index<TypeListOps::IndexOf<OriginalTypeList, TOriginal>()>;
        template <typename TWrapped>
        using WrappedIndexOf = Index<TypeListOps::IndexOf<WrappedTypeList, TWrapped>()>;
        //
        // ctors
        //
        constexpr variant()
            noexcept(std::is_nothrow_default_constructible<TypeListOps::FrontType<WrappedTypeList>>::value)
            : BaseType(Index<0>{}) { }
        
        variant(const variant &other) = default;
        variant(variant &&other) = default;

        template <typename U,
                  typename Enabled = std::enable_if_t<!std::is_same<std::decay_t<U>, variant>::value>,
                  typename Index = BestMatchIndex<WrappedTypeList, U>>
        constexpr variant(U &&rhs)
            noexcept(std::is_nothrow_constructible<TypeListOps::TypeAt<WrappedTypeList, Index::value>, U>::value)
            : BaseType(Index{}, std::forward<U>(rhs))
        { }
        template <typename T, typename ...TArgs>
        constexpr variant(in_place_type_t<T>, TArgs&& ...args)
            : BaseType(OriginalIndexOf<T>{}, std::forward<TArgs>(args)...) { }
        template <size_t I, typename ...TArgs>
        constexpr variant(in_place_index_t<I>, TArgs&& ...args)
            : BaseType(Index<I>{}, std::forward<TArgs>(args)...) { }

        ~variant() = default;

    public:
        constexpr size_t index() const noexcept
        {
            return TypeIndex();
        }

        constexpr bool valueless_by_exception() const noexcept
        {
            return TypeIndex() == variant_npos;
        }

        template <typename T, typename ...TArgs>
        void emplace(TArgs&& ...args)
        {
            emplace<>
                (std::forward<TArgs>(args)...);
        }
        template <size_t I, typename ...TArgs>
        void emplace(TArgs&& ...args)
        {
            Initialize<I>(std::forward<TArgs>(args)...);
        }
    };

    template <typename T, typename ...Ts>
    T &get(variant<Ts...> &v)
    {

    }
}