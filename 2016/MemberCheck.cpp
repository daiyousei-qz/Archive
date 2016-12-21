// technique to check if a member exist in a type

#include <type_traits>
#include <iostream>
#include <string>

using namespace std;

struct general_{};
struct special_ : general_{};
template <typename T>
struct int_ { using type = int; };

// if we check T::Register, behavior differs between compile-time and run-time
template <typename T>
constexpr bool check_internal_(typename int_<decltype(&std::declval<T>().Register)>::type, special_) { return true; }
template <typename T>
constexpr bool check_internal_(int, general_) { return false; }

template <typename T>
constexpr bool check()
{
	return check_internal_<T>(0, special_());
}

struct ItemA
{
	int Id;
	void Register() { }
};

struct ItemB { };

template <typename T>
void print()
{
	cout << (check<T>() ? "Register function found" : "Error") << endl;
}

template <typename T>
struct DisplayTypename;

enum class Type
{
	A,B,C
};

template <Type tval>
struct Test
{
	static constexpr Type type = tval;
};

int main()
{
	constexpr auto t = Test<Type::B>::type;

	print<ItemA>();
	print<ItemB>();
	constexpr int x = check<ItemA>() ? 41 : 7777;
	system("pause");
	return 0;
}

