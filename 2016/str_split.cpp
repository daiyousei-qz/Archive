#include <iostream>
#include <type_traits>

template <char ChValue>
struct AnsiChar
{
	static constexpr char Value = ChValue;
};

struct StrBase {};
struct StrTerminator : StrBase
{
	static constexpr size_t Length = 0;
	static constexpr char CurrentChar = '\0';

	static void Print(bool firstElem = true)
	{
		// if StrTerminator is the first element, just do nothing
		if(!firstElem)
			std::cout << '\"';
	}
};
struct ArrayBase {};
struct ArrayTerminator : ArrayBase 
{
	static constexpr size_t Length = 0;

	static void Print(bool firstElem = true)
	{
		// if ArrayTerminator is the first element, just do nothing
		if (!firstElem)
			std::cout << '}';
	}
};

template <typename T>
constexpr bool IsStr()
{
	return std::is_base_of<StrBase, T>::value;
}

template <typename T>
constexpr bool IsStrArray()
{
	return std::is_base_of<ArrayBase, T>::value;
}

template <typename FirstChar, typename ... RemainingChars>
struct Str : StrBase
{
	static_assert(std::is_same<FirstChar, AnsiChar<FirstChar::Value>>::value, "elem in a str should be a char");

	using TailingStr = Str<RemainingChars...>;

	static constexpr size_t Length = 1 + TailingStr::Length;
	static constexpr char CurrentChar = FirstChar::Value;

	static void Print(bool firstElem = true)
	{
		if (firstElem)
		{
			std::cout << '\"';
		}

		std::cout << CurrentChar;
		TailingStr::Print(false);
	}
};

template <typename Char>
struct Str<Char> : StrBase
{
	static_assert(std::is_same<Char, AnsiChar<Char::Value>>::value, "elem in a str should be a char");

	using TailingStr = StrTerminator;

	static constexpr char CurrentChar = Char::Value;
	static constexpr size_t Length = 1;

	static void Print(bool firstElem = true)
	{
		if (firstElem)
		{
			std::cout << '\"';
		}

		std::cout << CurrentChar;
		TailingStr::Print(false);
	}
};

// this is a special kind of Str used for concatenation
template <typename PrefixChar, typename RawString>
struct PrefixxedStr : StrBase
{
	static_assert(std::is_same<PrefixChar, AnsiChar<PrefixChar::Value>>::value, "elem in a str should be a char");
	static_assert(IsStr<RawString>(), "Non-Str type being prefixxed.");

	using TailingStr = RawString;

	static constexpr char CurrentChar = PrefixChar::Value;
	static constexpr size_t Length = 1 + RawString::Length;

	static void Print(bool firstElem = true)
	{
		if (firstElem)
		{
			std::cout << '\"';
		}

		std::cout << CurrentChar;
		TailingStr::Print(false);
	}
};

template <typename FirstStr, typename TailingArrayType>
struct StrArray : ArrayBase
{
	static_assert(IsStr<FirstStr>(), "elem in a StrArray should be a Str.");

	using CurrentStr = FirstStr;
	using TailingArray = TailingArrayType;

	static constexpr size_t Length = 1 + TailingArray::Length;

	static void Print(bool firstElem = true)
	{
		if (firstElem)
		{
			std::cout << '{';
		}
		else
		{
			std::cout << ',';
		}

		std::cout << ' ';
		CurrentStr::Print(true);
		TailingArray::Print(false);
	}
};

template <typename SourceString, typename TargetString>
struct TestStrHead
{
	static constexpr bool Result =
		SourceString::Length < TargetString::Length ? false :
		SourceString::CurrentChar != TargetString::CurrentChar ? false :
		TestStrHead<typename SourceString::TailingStr,
		typename TargetString::TailingStr>::Result;
};

template <typename SourceString>
struct TestStrHead<SourceString, StrTerminator> { static constexpr bool Result = true; };

template <>
struct TestStrHead<StrTerminator, StrTerminator> { static constexpr bool Result = true; };

template <typename RawString, size_t Size>
struct GetStrPrefix
{
	static_assert(Size <= RawString::Length, "size of prefix cannot exceed length of a Str.");

	using Result = PrefixxedStr<AnsiChar<RawString::CurrentChar>, 
								typename GetStrPrefix<typename RawString::TailingStr, Size - 1>::Result>;
};

template <typename RawString, size_t Offset>
struct GetSubStrAt
{
	static_assert(Offset > 0, "Index must be non-negative.");
	static_assert(Offset <= RawString::Length, "size of prefix cannot exceed length of a Str.");
	
	using Result = typename GetSubStrAt<typename RawString::TailingStr, Offset - 1>::Result;
};

template <typename RawString>
struct GetSubStrAt<RawString, 0>
{
	using Result = RawString;
};

template <typename RawString>
struct GetStrPrefix<RawString, 1>
{
	static_assert(1 <= RawString::Length, "size of prefix cannot exceed length of a Str.");

	using Result = PrefixxedStr<AnsiChar<RawString::CurrentChar>, StrTerminator>;
};

template <typename SourceString, typename TargetString, typename = void>
struct FindFirst
{
	using Result = StrTerminator;
};

template <typename SourceString, typename TargetString>
struct FindFirst<SourceString, TargetString, std::enable_if_t<SourceString::Length >= TargetString::Length>>
{
	using Result = std::conditional_t<
		TestStrHead<SourceString, TargetString>::Result,
		typename GetStrPrefix<SourceString, TargetString::Length>::Result,
		typename FindFirst<typename SourceString::TailingStr, TargetString>::Result>;
};

template <typename SourceString, typename TargetString, size_t Offset, typename = void>
struct IndexOf_Internal
{
	static constexpr size_t Result =
		TestStrHead<SourceString, TargetString>::Result ?
		Offset :
		IndexOf_Internal<typename SourceString::TailingStr, TargetString, Offset + 1>::Result;
};

template <typename SourceString, typename TargetString, size_t Offset>
struct IndexOf_Internal<SourceString, TargetString, Offset, std::enable_if_t<(SourceString::Length < TargetString::Length)>>
{
	static constexpr size_t Result = Offset + SourceString::Length;
};

template <typename SourceString, typename TargetString>
struct IndexOf
{
	static constexpr size_t Result = IndexOf_Internal<SourceString, TargetString, 0>::Result;
};

template <typename RawString, typename Separator, typename = void>
struct Split
{
	static constexpr size_t FirstSeparatorIndex = IndexOf<RawString, Str<Separator>>::Result;
	using Result = StrArray<typename GetStrPrefix<RawString, FirstSeparatorIndex>::Result,
							typename Split<typename GetSubStrAt<RawString, FirstSeparatorIndex + 1>::Result, Separator>::Result>;
};

template <typename RawString, typename Separator>
struct Split<RawString, 
			 Separator, 
			 std::enable_if_t<
				 std::is_same<
					 typename FindFirst<RawString, Str<Separator>>::Result,
					 StrTerminator>::value &&
				 RawString::Length != 0>>
{
	using Result = StrArray<RawString, ArrayTerminator>;
};

template <typename RawString, typename Separator>
struct Split<RawString, 
			 Separator, 
			 std::enable_if_t<RawString::Length == 0>>
{
	using Result = ArrayTerminator;
};

int main()
{
	using hw = Str <
		AnsiChar<'H'>,
		AnsiChar<'e'>,
		AnsiChar<'l'>,
		AnsiChar<'l'>,
		AnsiChar<'o'>,
		AnsiChar<' '>,
		AnsiChar<'W'>,
		AnsiChar<'o'>,
		AnsiChar<'r'>,
		AnsiChar<'l'>,
		AnsiChar<'d'>,
		AnsiChar<'!' >> ;

	using hw2 = Str <
		AnsiChar<'o'>,
		AnsiChar<'r'>> ;

	constexpr size_t sz = hw2::Length;
	constexpr bool x = TestStrHead<hw, hw2>::Result;
	FindFirst<hw,hw2>::Result::Print();
	using Array = typename Split<hw, AnsiChar<'o'>>::Result;
	Array::Print();

	return 0;
}