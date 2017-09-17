#include "Lexer.h"
#include "Error.h"
#include "TextUtils.h"
#include "TranslationContext.h"
#include <cassert>

using namespace std;

namespace lolita
{
	// Implementation of Lexer
	//

	void Lexer::PrepareCheckpoint(bool start_of_file)
	{
		// clear flags
		start_of_line_ = start_of_file;
		succeeding_space_ = false;
		hint_header_name_ = false;

		// skip any whitespace and comment
		while (!window_.empty())
		{
			auto ch = Peek();
			if (isspace(ch))
			{
				Consume();

				if (ch == '\n')
					start_of_line_ = true;
			}
			else if (ch == '/')
			{
				if (TrySeq("//"))
					SkipSingleLineComment();
				else if (TrySeq("/*"))
					SkipBlockComment();
				else
					break;
			}
			else
			{
				break;
			}

			succeeding_space_ = true;
		}

		// mark checkpoint
		dirty_ = false;
		checkpoint_loc_ = loc_;
		checkpoint_view_ = window_;
	}

	void Lexer::SkipSingleLineComment()
	{
		// assume "//" consumed
		while (TryPred([](char ch) { return ch != '\n'; })) {}
	}

	void Lexer::SkipBlockComment()
	{
		// assume "/*" consumed

		while (!Exhausted())
		{
			if (Peek() == '*' && TrySeq("*/"))
				return;

			// consume one to look at the next char
			Consume();
		}

		// block comment doesn't terminate correctly
		// this could be devestating
		ReportError(loc_, "block comment not closed");
		AbortTranslation();
	}

	Token Lexer::LexNext()
	{
		assert(!Exhausted());

		// Lex the next token according to the next char
		switch (char ch = Consume())
		{
			// Brackets
			//
		case '(':
			return ExportToken(TokenTag::LParenthesis);
		case ')':
			return ExportToken(TokenTag::RParenthesis);
		case '[':
			return ExportToken(TokenTag::LBracket);
		case ']':
			return ExportToken(TokenTag::RBracket);
		case '{':
			return ExportToken(TokenTag::LBrace);
		case '}':
			return ExportToken(TokenTag::RBrace);

			// Operators ans other Symbols
			//
		case '=':
			if (Try('='))
				return ExportToken(TokenTag::Equal);
			else
				return ExportToken(TokenTag::Assign);

		case '+':
			if (Try('='))
				return ExportToken(TokenTag::PlusAssign);
			else if (Try('+'))
				return ExportToken(TokenTag::Increment);
			else
				return ExportToken(TokenTag::Plus);

		case '-':
			if (Try('='))
				return ExportToken(TokenTag::MinusAssign);
			else if (Try('-'))
				return ExportToken(TokenTag::Decrement);
			else if (Try('>'))
				return ExportToken(TokenTag::Arrow);
			else
				return ExportToken(TokenTag::Minus);

		case '*':
			if (Try('='))
				return ExportToken(TokenTag::MulAssign);
			else
				return ExportToken(TokenTag::Asterisk);

		case '/':
			if (Try('='))
				return ExportToken(TokenTag::DivAssign);
			else
				return ExportToken(TokenTag::Slash);

		case '%':
			if (Try('='))
				return ExportToken(TokenTag::ModAssign);
			else
				return ExportToken(TokenTag::Modulus);

		case '&':
			if (Try('='))
				return ExportToken(TokenTag::BitAndAssign);
			else if (Try('&'))
				return ExportToken(TokenTag::DoubleAmp);
			else
				return ExportToken(TokenTag::Ampersand);

		case '|':
			if (Try('='))
				return ExportToken(TokenTag::BitOrAssign);
			else if (Try('|'))
				return ExportToken(TokenTag::DoublePipe);
			else
				return ExportToken(TokenTag::Pipe);

		case '^':
			if (Try('='))
				return ExportToken(TokenTag::BitXorAssign);
			else
				return ExportToken(TokenTag::Caret);

		case '~':
			return ExportToken(TokenTag::Tlide);

		case '<':
			if (hint_header_name_)
				return LexHeaderName(true);
			if (Try('='))
				return ExportToken(TokenTag::LessEqual);
			else if (Try('<'))
				if (Try('='))
					return ExportToken(TokenTag::LShiftAssign);
				else
					return ExportToken(TokenTag::LShift);
			else
				return ExportToken(TokenTag::Less);

		case '>':
			if (Try('='))
				return ExportToken(TokenTag::GreaterEqual);
			else if (Try('>'))
				if (Try('='))
					return ExportToken(TokenTag::RShiftAssign);
				else
					return ExportToken(TokenTag::RShift);
			else
				return ExportToken(TokenTag::Greater);

		case '!':
			return ExportToken(TokenTag::Exclamation);

		case '.':
			if (TryPred(isdigit))
				return LexPPNumber(false);
			else if (TrySeq(".."))
				return ExportToken(TokenTag::Ellipsis);
			else
				return ExportToken(TokenTag::Period);

		case '?':
			return ExportToken(TokenTag::Question);
		case ':':
			return ExportToken(TokenTag::Colon);
		case ';':
			return ExportToken(TokenTag::Semicolon);
		case ',':
			return ExportToken(TokenTag::Comma);

			// Constants and Literals
			//
		case '\'':
			return LexCharConst();

		case '\"':
			if (hint_header_name_)
				return LexHeaderName(false);
			else
				return LexStringLiteral();

		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
		case '8': case '9':
			return LexPPNumber(true);

		case 'U':
			if (Try('\''))
				return LexCharConst();
			else if (Try('\"'))
				return LexStringLiteral();
			else
				return LexIdentifier();

		case 'u':
			if (TrySeq("8\'") || Try('\''))
				return LexCharConst();
			else if (TrySeq("8\"") || Try('\"'))
				return LexStringLiteral();
			else
				return LexIdentifier();
			break;

			// Identifier
			//
		case 'A': case 'B': case 'C': case 'D':
		case 'E': case 'F': case 'G': case 'H':
		case 'I': case 'J': case 'K': case 'L':
		case 'M': case 'N': case 'O': case 'P':
		case 'Q': case 'R': case 'S': case 'T':
		/* 'U' */ case 'V': case 'W': case 'X':
		case 'Y': case 'Z':
		case 'a': case 'b': case 'c': case 'd':
		case 'e': case 'f': case 'g': case 'h':
		case 'i': case 'j': case 'k': case 'l':
		case 'm': case 'n': case 'o': case 'p':
		case 'q': case 'r': case 's': case 't':
		/* 'u' */ case 'v': case 'w': case 'x':
		case 'y': case 'z':

		case '_':
			return LexIdentifier();

			//
			// Preprocessing Symbols
			//
		case '#':
			if (Try('#'))
				return ExportToken(TokenTag::SharpSharp);
			else
				return ExportToken(TokenTag::Sharp);

		default:
			// as we consider any non-ASCII character part of an identifier
			// recheck if ch starts an identifier
			if (ch > 0x7f)
			{
				return LexIdentifier();
			}
			else
			{
				return ExportToken(TokenTag::InvalidChar);
			}
		}
	}

	// in C standard, actually a ppnumber should be lexed
	// but we may parse it in advance, which is equvilant
	Token Lexer::LexPPNumber(bool int_hint)
	{
		while (!window_.empty())
		{
			if (Try('.'))
			{
				int_hint = false;
				continue;
			}
			else if (TryAny("eEpP"))
			{
				TryAny("+-");

				int_hint = false;
				continue;
			}
			else if (TryPred(isalnum))
			{
				continue;
			}

			break;
		}

		return ExportToken(int_hint ? TokenTag::IntegerConst : TokenTag::FloatConst);
	}

	Token Lexer::LexCharConst()
	{
		// assume prefix and opening single-quote consumed

		// iterate to skip body and closing quote
		while (!window_.empty() && Peek() != '\n')
		{
			if (Try('\''))
			{
				return ExportToken(TokenTag::CharConst);
			}
			else
			{
				// consume next char if an escape sequence is followed
				// it's okay to only skip one character
				Try('\\');
				Consume();
			}
		}

		// error, but ignore it
		return ExportToken(TokenTag::CharConst);
	}

	Token Lexer::LexStringLiteral()
	{
		// assume prefix and double-quote consumed

		// iterate to skip body and closing quote
		while (!Exhausted() && Peek() != '\n')
		{
			if (Try('\"'))
			{
				return ExportToken(TokenTag::StringLiteral);
			}
			else
			{
				// consume next char is safe for an escape sequence
				Try('\\');
				Consume();
			}
		}

		// error, but ignore it
		return ExportToken(TokenTag::StringLiteral);
	}

	Token Lexer::LexIdentifier()
	{
		// assume first character consumed

		while (char ch = Peek())
		{
			if (ch == '_' || isalnum(ch) || ch & 0x80)
				Consume();
			else
				break;
		}

		return ExportToken(TokenTag::Identifier);
	}

	Token Lexer::LexHeaderName(bool angle)
	{
		// assume opening symbol consumed

		// select closing symbol
		const auto delimit = angle ? '>' : '"';

		// iterate to skip body and closing symbol
		while (!Exhausted() && Peek() != '\n')
		{
			if (Try(delimit))
			{
				return ExportToken(TokenTag::HeaderName);
			}
			else
			{
				// NOTE no escape sequence is allowed in a HeaderName
				Consume();
			}
		}

		// error
		return ExportToken(TokenTag::BadHeaderName);
	}

	Token Lexer::ExportToken(TokenTag tag)
	{
		auto consumed_len = checkpoint_view_.length() - window_.length();
		auto consumed_view = checkpoint_view_.substr(0, consumed_len);

		if (dirty_)
		{
			// cleanup dirty content
			auto buf = string{};
			bool escaped = false;
			for (auto ch : consumed_view)
			{
				if (escaped && ch == '\n')
					buf.pop_back();
				else
					buf.push_back(ch);

				escaped = (ch == '\\');
			}

			return Token{ tag, buf, checkpoint_loc_, start_of_line_, succeeding_space_ };
		}
		else
		{
			return Token{ tag, string{ consumed_view }, checkpoint_loc_, start_of_line_, succeeding_space_ };
		}
	}

	char Lexer::Consume()
	{
		// consume the next character
		char result = window_.front();
		window_.remove_prefix(1);

		// update location
		loc_.Index += 1;
		if (result == '\n')
		{
			loc_.Column = 1;
			loc_.Line += 1;
		}
		else
		{
			loc_.Column += (result == '\t' ? kLexerTabSize : 1);
		}

		// prepare for next consumption
		// this eliminate checking in Peek()
		SkipLineConnect();

		// yield what is consumed
		return result;
	}
}