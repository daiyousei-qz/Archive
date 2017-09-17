#pragma once
#include "Token.h"
#include "TranslationContext.h"
#include "MacroEngine.h"
#include <deque>
#include <memory>

namespace lolita
{
	enum class PPDirective
	{
		Null,
		Unknown,

		If,
		IfDef,
		IfNDef,
		Else,
		ElseIf,
		EndIf,
		Define,
		Undef,
		Include,
		Pragma,
		Line,
		Error
	};

	class Preprocessor
	{
	public:
		Preprocessor() { }

		TokenVec Preprocess(const TokenVec& input)
		{
			// preprocess the file
			PreprocessInternal(input);
			// keep the EOF token
			buffer_.emplace_back(input.back());

			return std::move(buffer_);
		}

	private:
		void PreprocessInternal(const TokenVec& input);
		void FinalizeToken(const Token& tok);


		void ExecutePP(TokenSource& src);
		void ExecutePPIf(TokenSource& src, SourceLocation loc);
		void ExecutePPIfDef(TokenSource& src, SourceLocation loc, bool expect_def);
		void ExecutePPElse(TokenSource& src, SourceLocation loc);
		void ExecutePPElif(TokenSource& src, SourceLocation loc);
		void ExecutePPEndIf(TokenSource& src, SourceLocation loc);
		void ExecutePPDefine(TokenSource& src, SourceLocation loc);
		void ExecutePPUndef(TokenSource& src, SourceLocation loc);
		void ExecutePPInclude(TokenSource& src, SourceLocation loc);
		void ExecutePPPragma(TokenSource& src, SourceLocation loc);
		void ExecutePPLine(TokenSource& src, SourceLocation loc);
		void ExecutePPError(TokenSource& src, SourceLocation loc);

		bool PPTryExpandMacro(TokenSource& src)
		{
			assert(src.Test(TokenTag::Identifier));
			auto iter = macros_.find(src.Peek().Content);
			if (iter != macros_.end())
			{
				// there is a macro of this name
				// maybe a macro expansion
				auto obj_like = iter->second.ObjectLike;
				if (!obj_like)
				{
					// i.e. function-like
					// a strictly followed '(' required to form a function-like macro
					auto next_tok = src.LookAhead(1);
					if (next_tok.Tag != TokenTag::LParenthesis
						|| next_tok.SucceedingSpace)
					{
						return false;
					}
				}

				for (const auto& tok : ExpandMacro(src, macros_, obj_like))
				{
					FinalizeToken(tok);
				}

				return true;
			}
			else
			{
				return false;
			}
		}

		TokenVec PPExtractLine(TokenSource& src)
		{
			auto result = TokenSeq{};
			while (!src.StartOfLine())
			{
				result.emplace_back(src.Consume());
			}
			result.emplace_back(
				Token{ TokenTag::EndOfFile,{}, src.Peek().Location, true, true });

			return result;
		}

		bool PPExpectTag(TokenSource& src, TokenTag tag)
		{
			if (src.StartOfLine())
			{
				ReportUnexpectedNewline(src.Peek());
				return false;
			}
			else if (src.Try(tag))
			{
				return true;
			}
			else
			{
				ReportUnexpectedToken(src.Peek(), tag);
				return false;
			}
		}
		// when a preprocessing directive hit a parsing error
		// skip the line to remove redundant error message
		void PPFinalizeSource(TokenSource& src)
		{
			if (!src.StartOfLine())
				src.OmitCurrentLine();
		}

	private:
		MacroArchive macros_;

		int line_offset_;
		size_t depth_;

		TokenVec buffer_;
	};
}