#include "LiteralParser.h"
#include "Error.h"
#include "TextUtils.h"
#include <vector>
#include <locale>
#include <codecvt>
#include <string_view>

using namespace std;

namespace lolita
{
	//
	// Helper Functions
	//
	bool IsOctalDigit(char ch)
	{
		return ch >= '0' && ch <= '7';
	}

	bool IsDecimalDigit(char ch)
	{
		return ch >= '0' && ch <= '9';
	}

	bool IsHexDigit(char ch)
	{
		return (ch >= '0' && ch <= '9')
			|| (ch >= 'a' && ch <= 'f')
			|| (ch >= 'A' && ch <= 'F');
	}

	int HexDigitToValue(char ch)
	{
		if (ch >= '0' && ch <= '9')
			return ch - '0';
		else if (ch >= 'a' && ch <= 'f')
			return ch - 'a' + 10;
		else if (ch >= 'A' && ch <= 'F')
			return ch - 'A' + 10;

		// not a hexadecimal digit
		assert(false);
		return 0; // make the compiler happy
	}

	//
	// Parsing a numeric constant in C
	//
	// [Lexical Definition]
	// NumericConst			::= IntConst | FloatConst
	//
	// IntConst				::= (OctalIntConst | DecimalIntConst | HexIntConst) IntSuffix
	// OctalIntConst		::= "0[0-7]*"
	// DecimalIntConst		::= "[1-9][0-9]*"
	// HexIntConst			::= "0x[0-9a-fA-F]+"
	// IntSuffix			::= (UnsignIntSuffix LongIntSuffix?) | (LongIntSuffix UnsignIntSuffix?)
	// UnsignIntSuffix		::= "u|U"
	// LongIntSuffix		::= "l|ll|L|LL"
	//
	// FloatConst			::= (DecimalFloatConst | HexFloatConst) FloatSuffix
	// DecimalFloatConst	::= DecimalSignificand DecimalExponent
	// DecimalSignificand   ::= (DecimalDigitSeq "." DecimalDigitSeq?) | (DecimalDigitSeq? "." DecimalDigitSeq)
	// DecimalExponent		::= "[eE][+-]?" DecimalDigitSeq
	// DecimalDigitSeq		::= "[0-9]+"
	// HexFloatConst		::= "0x" HexSignificand HexExponent
	// HexSignificand		::= (HexDigitSeq "." HexDigitSeq?) | (HexDigitSeq? "." HexDigitSeq)
	// HexExponent			::= "[pP][+-]?" DecimalDigitSeq
	// DecimalDigitSeq		::= "[0-9a-fA-F]+"
	// FloatSuffix			::= "f|F|l|L"
	//
	// [Note]
	// - A numeric constant is always positive
	//===========================================================================

	// NOTE long_ and longlong_ should not be true simultaneously
	IntPrecision DetermineIntPrecision(bool unsign_, bool long_, bool longlong_)
	{
		static constexpr IntPrecision typemap[][2] =
		{
			{ IntPrecision::Int32, IntPrecision::UInt32 },
			{ IntPrecision::Int64, IntPrecision::UInt64 },
			{ IntPrecision::Int64, IntPrecision::UInt64 },
		};

		assert(!(long_&&longlong_));
		auto type_select = 0;
		if (long_) type_select = 1;
		if (longlong_) type_select = 2;

		return typemap[type_select][unsign_ ? 1 : 0];
	}

	CInteger ParseIntegerConst(const Token& tok)
	{
		// NOTE that integer constant is always positive
		// (sign would be parsed saperately)
		assert(tok.Tag == TokenTag::IntegerConst);

		// parse value
		// FIXME: handle possible exceptions
		size_t pos;
		auto value = stoull(tok.Content, &pos, 0);

		// parse type suffix
		auto prec = IntPrecision::Int32;
		if (pos != tok.Content.size())
		{
			auto unsigned_ = false;
			auto long_ = false;
			auto longlong_ = false;

			auto view = string_view{ tok.Content.c_str() + pos };

			// first round
			unsigned_ = TryAnyPrefix(view, "uU");
			longlong_ = TrySeqPrefix(view, "ll") || TrySeqPrefix(view, "LL");

			if (!longlong_)
				long_ = TryAnyPrefix(view, "lL");

			// second round
			if (!unsigned_)
			{
				unsigned_ = TryAnyPrefix(view, "uU");
			}

			if (!long_ && !longlong_)
			{
				longlong_ = TrySeqPrefix(view, "ll") || TrySeqPrefix(view, "LL");

				if (!longlong_)
					long_ = TryAnyPrefix(view, "lL");
			}

			// invalid suffix for integer
			if (!view.empty())
			{
				throw LiteralError{ };
			}

			prec = DetermineIntPrecision(unsigned_, long_, longlong_);
		}

		// finish parsing
		// FIXME: test overflow

		return CInteger{ value, prec };
	}

	CFloat ParseFloatConst(const Token& tok)
	{
		assert(tok.Tag == TokenTag::FloatConst);

		// parse value
		// FIXME: handle possible exceptions
		size_t pos = 0;
		auto value = stod(tok.Content, &pos);

		// parse type suffix
		auto prec = FloatPrecision::Double;
		if (pos != tok.Content.size())
		{
			auto view = string_view{ tok.Content.c_str() + pos };

			if (TryAnyPrefix(view, "fF"))
			{
				prec = FloatPrecision::Single;
			}

			// FIXME: add long double support

			// invalid suffix for integer
			if (!view.empty())
			{
				throw LiteralError{ };
			}
		}

		return CFloat{ value, prec };
	}

	//
	// Parsing character constant in C
	//
	// [Lexical Definition]
	// CharConst ::= (TypeSpecifier SQuote CChar SQuote) | (SQuote CCharSeq SQuote)
	// TypeSpecifier ::= "u|U|L"
	// SQuote ::= "'"
	// CChar ::= "[^'\\]" | EscapedChar | EscapedSeq | UCN
	// EscapedChar ::= "\\'|\\"|\\?|\\\\|\\a|\\b|\\f|\\n|\\r|\\t|\\v"
	// EscapedSeq ::= "\\x[0-9a-fA-F]{2}" | "\\[0-7]{3}"
	// UCN ::= "\\u[0-9a-fA-F]{8}" | "\\u[0-9a-fA-F]{4}"

	uint32_t ReadMaybeEscaped(string_view &view, uint32_t char_max)
	{
		// Without a backslash, raw character should be read
		if (!TryPrefix(view, '\\'))
		{
			return ConsumeFront(view);
		}

		switch (auto ch = ConsumeFront(view))
		{
			// non-printables
		case 'a':
			return '\a';
		case 'b':
			return '\b';
		case 'f':
			return '\f';
		case 'n':
			return '\n';
		case 'r':
			return '\r';
		case 't':
			return '\t';
		case 'v':
			return '\v';

			// specific value
		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
		{
			// \nnn where n is octal digit

			uint32_t result = ch - '0';

			for (int i = 0; i < 2; ++i)
			{
				char next = view.front();
				if (IsOctalDigit(next))
				{
					result *= 8;
					result += next - '0';

					view.remove_prefix(1);
				}
			}

			return result;
		}

		case 'x':
		{
			// \xnn where n is hexadecimal digit
			// NOTE digit sequence only terminates when input exhausts or non-digit character is hit

			int i = 0;
			uint32_t result = 0;
			for (; !view.empty(); ++i)
			{
				char next = view.front();
				if (IsHexDigit(next))
				{
					result *= 16;
					result += HexDigitToValue(next);

					view.remove_prefix(1);
				}
			}

			if (i == 0)
			{
				// No digit consumed
				throw LiteralError{ };
			}
			else if (result > char_max)
			{
				// non-UCN character too large for the character type
				throw LiteralError{ };
			}
			else
			{
				return result;
			}
		}

		// FIXME: UCN not prior to be implemented
		/*
		case 'u':
		{
		// \unnnn or \unnnnnnnn where n is hexadecimal digit


		}
		//*/

		default:
			if (ch != '\0')
				return ch;
			else
				throw LiteralError{ };
		}
	}

	CInteger ParseCharConst(const Token& tok)
	{
		auto view = string_view{ tok.Content };

		// FIXME: Magic Number used: assumption on character types
		// parse prefix
		auto type = IntPrecision::UInt8;
		uint32_t char_max = 0xffU;
		uint32_t char_len = 1;
		if (TryPrefix(view, 'u') || TryPrefix(view, 'L'))
		{
			type = IntPrecision::UInt16;
			char_max = 0xffffU;
			char_len = 2;
		}
		else if (TryPrefix(view, 'U'))
		{
			type = IntPrecision::UInt32;
			char_max = 0xffffffffU;
			char_len = 4;
		}

		// parse leading single quote
		if (!TryPrefix(view, '\''))
		{
			throw LiteralError{ };
		}

		// FIXME: Magic Number used: 4 indicate size of int
		// parse char seq
		int i = 0;
		uint32_t result = 0;
		for (; i < 4 / char_len; ++i)
		{
			if (view.front() == '\'')
			{
				// single quote, char-const closed
				break;
			}

			if (i > 0)
			{
				// this is a multi-character sequence
				type = IntPrecision::Int32;
			}

			auto ch = ReadMaybeEscaped(view, char_max);
			// if value is larger than the character type can represent(caused by UCN)
			// it should be truncated
			result <<= char_len * 8;
			result |= char_max & ch;
		}

		if (i == 0)
		{
			// empty char-const
			throw LiteralError{ };
		}
		if (!TryPrefix(view, '\''))
		{
			// maybe too many characters in definition
			throw LiteralError{ };
		}

		return CInteger(result, type);
	}

	//
	// Parsing string literal in C
	//
	CString ParseStringLiteral(const Token& tok)
	{
		auto view = string_view{ tok.Content };

#pragma warning "Magic Number used: assumption on character types"
		// parse prefix
		// NOTE project lolita specify utf-8 as its execution character set
		// so actually u8 prefix doesn't have any effect
		EncodingType type = EncodingType::Char;
		uint32_t char_max = 0xffU;
		uint32_t char_len = 1;
		if (TryPrefix(view, 'u'))
		{
			if (!TryPrefix(view, '8'))
			{
				type = EncodingType::Char16;
				char_max = 0xffffU;
				char_len = 2;
			}
		}
		else if (TryPrefix(view, 'U'))
		{
			type = EncodingType::Char32;
			char_max = 0xffffffffU;
			char_len = 4;
		}
		else if (TryPrefix(view, 'L'))
		{
			type = EncodingType::WChar;
			char_max = 0xffffU;
			char_len = 2;
		}

		// parse leading double quote
		if (!TryPrefix(view, '\"'))
		{
			throw LiteralError{ };
		}

		// parse contents(use UCS4 as intermediate encoding)
		basic_string<uint32_t> buffer32; // should be u32string, workaround for MSVC
		while (!view.empty())
		{
			if (view.front() == '\"')
			{
				// double quote, char-const closed
				break;
			}

			uint32_t ch = ReadMaybeEscaped(view, char_max);
			buffer32.push_back(ch);
		}

		// parse closing double quote
		if (!TryPrefix(view, '\"'))
		{
			throw LiteralError{ };
		}

#ifdef _MSC_VER
		// workaround: MSVC cannot handle the correct form of cvt definition
		wstring_convert<codecvt_utf8<uint32_t>, uint32_t> cvt;
#else
		wstring_convert<codecvt_utf8<char32_t>, char32_t> cvt;
#endif

		return CString(cvt.to_bytes(buffer32), type);
	}
}