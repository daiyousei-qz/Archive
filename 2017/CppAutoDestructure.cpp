#include <type_traits>
#include <tuple>
#include <cassert>

struct AnyType
{
	template<typename T>
	constexpr operator T() const;
};

template<typename T, typename... Anys>
decltype(void(T{ std::declval<Anys>()... }), std::true_type{}) ConstructibleTest(int);

template<typename, typename...>
std::false_type ConstructibleTest(std::nullptr_t);

template<typename T, typename... Anys>
using IsConstructible = decltype(ConstructibleTest<T, Anys...>(0));

template<typename RefType, typename... Ts>
constexpr auto CreateTuple(Ts&& ...values)
{
	if constexpr(std::is_lvalue_reference_v<RefType>)
	{
		// lvalue -> reference them
		return std::tuple<Ts&...>{values...};
	}
	else
	{
		// rvalue -> move them
		return std::tuple<std::remove_reference_t<Ts>...>{ std::move(values)... };
	}
}

template<typename T>
constexpr auto TupleBinding(T&& ins)
{
	using RawType = std::decay_t<T>;

	if constexpr(IsConstructible<RawType, AnyType, AnyType, AnyType, AnyType>::value)
	{
		auto&[x1, x2, x3, x4] = ins;
		return CreateTuple<T&&>(x1, x2, x3, x4);
	}
	else if constexpr(IsConstructible<RawType, AnyType, AnyType, AnyType>::value)
	{
		auto&[x1, x2, x3] = ins;
		return CreateTuple<T&&>(x1, x2, x3);
	}
	else if constexpr(IsConstructible<RawType, AnyType, AnyType>::value)
	{
		auto&[x1, x2] = ins;
		return CreateTuple<T&&>(x1, x2);
	}
	else if constexpr(IsConstructible<RawType, AnyType>::value)
	{
		auto&[x1] = ins;
		return CreateTuple<T&&>(x1);
	}
	else
	{
		return CreateTuple<T&&>();
	}
}

struct Point
{
	int x = 41, y = 42;
};

int main() 
{
	using namespace std;
	Point b;

	auto x = TupleBinding(Point{});
	get<0>(x) = 13;
	get<1>(x) = 17;
	assert(get<0>(x) == 13 && get<1>(x) == 17);
	assert(b.x == 41 && b.y == 42);

	auto y = TupleBinding(b);
	get<0>(y) = 13;
	get<1>(y) = 17;
	assert(get<0>(y) == 13 && get<1>(y) == 17);
	assert(b.x == 13 && b.y == 17);
}