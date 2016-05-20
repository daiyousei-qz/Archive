#include <iostream>
#include <cstdlib>

template <size_t Offset>
struct _EndingArrayElement
{
    static constexpr size_t Index = Offset;
    using SelfElement = _EndingArrayElement;
    using NextElement = _EndingArrayElement;
    using TypeValue = void;
};

template <size_t Offset, typename T, typename ... TArgs>
struct _ArrayElement
{
    static_assert(!std::is_void<T>::value, "element in array cannot be void");

    static constexpr size_t Index = Offset;
    using SelfElement = _ArrayElement<Offset, T, TArgs...>;
    using NextElement = _ArrayElement<Offset + 1, TArgs...>;
    using TypeValue = T;

    template <size_t N> 
    using At = std::conditional_t<N == Index, SelfElement, typename NextElement::template At<N>>;

    template <typename U>
    static constexpr size_t IndexOf()
    {
        return std::is_same<TypeValue, U>::value ? Index : NextElement::IndexOf<U>();
    }
};

template <size_t Offset, typename T>
struct _ArrayElement<Offset, T>
{
    static_assert(!std::is_void<T>::value, "element in array cannot be void");

    static constexpr size_t Index = Offset;
    using SelfElement = _ArrayElement<Offset, T>;
    using NextElement = _EndingArrayElement<Offset + 1>;
    using TypeValue = T;

    template <size_t N> 
    using At = std::conditional_t<N == Index, SelfElement, NextElement>;

    template <typename U>
    static constexpr size_t IndexOf()
    {
        return std::is_same<TypeValue, U>::value ? Index : -1;
    }
};

template <typename ... TArgs>
struct _Array
{
    template <size_t N>
    using At = typename _ArrayElement<0, TArgs...>::template At<N>;

    template <size_t N>
    using TypeAt = typename At<N>::TypeValue;

    template <typename U>
    static constexpr size_t IndexOf()
    {
        return At<0>::IndexOf<U>();
    }
};

int main()
{
    using arr_t = _Array<int, float, char*>;
    constexpr size_t x = arr_t::IndexOf<float>();
    constexpr typename arr_t::TypeAt<x> y = 0;

    system("pause");
    return 0;
}