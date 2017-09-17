#pragma once
#include "SourceLocation.h"
#include <string>
#include <vector>
#include <cassert>
#include <algorithm>

namespace lolita
{
	enum class TokenTag
	{
		// Preprocessing Temporaries
		//

		// the last token of a source file
		EndOfFile,

		// 
		HeaderName,

		// a temporary token to simplify macro expansion
		Placemarker,

		// #
		Sharp,

		// ##
		SharpSharp,

		InvalidChar,

		BadHeaderName,

		// Punctuators
		//

		/* Bracket */
		LParenthesis,	// (
		RParenthesis,	// )
		LBracket,		// [
		RBracket,		// ]
		LBrace,			// {
		RBrace,			// }

		/* Assignment Operator */
		Assign,			// =
		PlusAssign,		// +=
		MinusAssign,	// -=
		MulAssign,		// *=
		DivAssign,		// /=
		ModAssign,		// %=
		BitAndAssign,	// &=
		BitOrAssign,	// |=
		BitXorAssign,	// ^=
		LShiftAssign,	// <<=
		RShiftAssign,	// >>=

		/* Inc\Dec Operator */
		Increment,		// ++
		Decrement,		// --

		/* Arithmetic Operator */
		Plus,			// +
		Minus,			// -
		Asterisk,		// *
		Slash,			// /
		Modulus,		// %
		Ampersand,		// &
		Pipe,			// |
		Caret,			// ^
		Tlide,			// ~
		LShift,			// <<
		RShift,			// >>

		/* Logical Operator */
		Exclamation,	// !
		DoubleAmp,		// &&
		DoublePipe,		// ||

		/* Comparison Operator */
		Equal,			// ==
		Unequal,		// !=
		Less,			// <
		Greater,		// >
		LessEqual,		// <=
		GreaterEqual,	// >=

		/* Other Puctuations */
		Period,			// .
		Ellipsis,       // ...
		Arrow,			// ->
		Question,		// ?
		Colon,			// :
		Semicolon,		// ;
		Comma,			// ,

		// Keywords
		//

		Alignas,		// _Alignas
		Alignof,		// _Alignof
		Atomic,			// _Atomic
		Auto,			// auto
		Break,			// break
		Bool,			// _Bool
		Case,			// case
		Char,			// char
		Complex,		// _Complex
		Const,			// const
		Continue,		// continue
		Default,		// default
		Do,				// do
		Double,			// double
		Else,			// else
		Enum,			// enum
		Extern,			// extern
		Float,			// float
		For,			// for
		Generic,		// _Generic
		Goto,			// goto
		If,				// if
		Imaginary,		// _Imaginary
		Inline,			// inline
		Int,			// int
		Long,			// long
		Noreturn,		// _Noreturn
		Register,		// register
		Restrict,		// restrict
		Return,			// return
		Short,			// short
		Signed,			// signed
		Sizeof,			// sizeof
		Static,			// static
		StaticAssert,	// _Static_assert
		Struct,			// struct
		Switch,			// switch
		ThreadLocal,	// _Thread_local
		Typedef,		// typedef
		Union,			// union
		Unsigned,		// unsigned
		Void,			// void
		Volatile,		// volatile
		While,			// while

		// Non-deterministic Tokens
		//
		Identifier,
		IntegerConst,
		FloatConst,
		CharConst,
		StringLiteral,
	};

	struct Token
	{
		// category of the token
		TokenTag Tag;

		// text form of the token
		std::string Content;

		// reference location of the token
		SourceLocation Location;

		// indicates if this token is the first non-whitespace token in the line
		bool StartOfLine;

		// indicates if there's any whitespace preceeding this token
		bool SucceedingSpace;
	};

	// a TokenVec instance should always ends with an EndOfFile Token
	using TokenVec = std::vector<Token>;

	// a wrapper class for TokenSeq that provides iteration utilities
	class TokenSource
	{
	public:
		TokenSource(const TokenVec& seq)
			: src_(seq), index_(0)
		{ 
			assert(!seq.empty());
			assert(seq.back().Tag == TokenTag::EndOfFile);
			assert(seq.back().StartOfLine);
		}

		bool Exhausted() const
		{
			if (Peek().Tag == TokenTag::EndOfFile)
			{
				assert(index_ == src_.size() - 1);
			}

			return Peek().Tag == TokenTag::EndOfFile;
		}

		const Token& Peek() const
		{
			return src_[index_];
		}

		const Token& LookAhead(size_t step) const
		{
			auto forward_index = 
				std::min(index_ + step, src_.size() - 1);

			return src_[forward_index];
		}

		const Token& Consume()
		{
			const auto& result = Peek();
			index_ += Exhausted() ? 0 : 1;

			return result;
		}

		const Token& LastConsumed()
		{
			if (index_ == 0)
				return src_[0];
			else
				return src_[index_ - 1];
		}

		void OmitCurrentLine()
		{
			while (!StartOfLine())
				Consume();
		}

		bool StartOfLine() const
		{
			return Peek().StartOfLine;
		}

		bool Test(TokenTag tag) const
		{
			return Peek().Tag == tag;
		}

		bool Try(TokenTag tag)
		{
			if (Test(tag))
			{
				Consume();
				return true;
			}

			return false;
		}

	private:
		const TokenVec& src_;
		size_t index_;
	};
}