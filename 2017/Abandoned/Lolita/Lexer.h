#pragma once
#include "CompilerConfig.h"
#include "Token.h"
#include "SourceLocation.h"
#include "TextUtils.h"
#include <string>
#include <string_view>
#include <algorithm>
#include <cassert>

namespace lolita
{
	// A forward lexer that decomposes source text into string of tokens
	// NOTE class Lexer is unaware of the preprocessing context
	// and relies on external hint to parse a HeaderName
	class Lexer
	{
	public:
		Lexer(std::string_view dat, const SourceFile* file)
			: checkpoint_view_(dat)
			, window_(dat)
		{
			// initialize location track
			loc_.Index = 0;
			loc_.Column = loc_.Line = 1;
			loc_.File = file;

			// prepare consumption
			SkipLineConnect();

			// prepare lexing
			PrepareCheckpoint(true);
		}

		bool Exhausted() const
		{
			return checkpoint_view_.empty();
		}

		Token Next()
		{
			if (Exhausted())
				return Token{ TokenTag::EndOfFile,{}, loc_, true, true };

			// cache result
			auto result = LexNext();
			// prepare lexing next token
			PrepareCheckpoint(false);

			return result;
		}

		// Backdoor for context control
		//

		// this would make Lexer try to make next token a HeaderName
		void HintHeaderName()
		{
			hint_header_name_ = true;
		}

	private:

		// Lex Operation
		//

		// call PrepareCheckpoint() to prepare for lexing next token
		// a CHECKPOINT should be where a valid token or EOF exactly start
		void PrepareCheckpoint(bool start_of_file);
		void SkipSingleLineComment();
		void SkipBlockComment();

		// lex next token
		// this function assume a checkpoint is correctly marked
		Token LexNext();

		Token LexPPNumber(bool int_hint);
		Token LexCharConst();
		Token LexStringLiteral();
		Token LexIdentifier();
		Token LexHeaderName(bool angle);

		// export consumed content from last checkpoint with particular token tag
		Token ExportToken(TokenTag tag);

		// Text Utils
		//

		void SkipLineConnect();
		char Peek() const;
		char Consume();

		template <typename Predicate>
		bool TryPred(Predicate pred);

		bool Try(char ch);
		bool TryAny(std::string_view any);
		bool TrySeq(std::string_view seq);

	private:
		// location at start of checkpoint
		SourceLocation checkpoint_loc_;
		// view to checkpoint
		std::string_view checkpoint_view_;

		// flags for checkpoint
		//
		bool start_of_line_;
		bool succeeding_space_;
		bool hint_header_name_;

		// flags for consumed
		bool dirty_;

		// location at start of view
		SourceLocation loc_;
		// view to remaining source text
		std::string_view window_;
	};

	inline void Lexer::SkipLineConnect()
	{
		while (TrySeqPrefix(window_, "\\\n"))
			dirty_ = true;
	}

	inline char Lexer::Peek() const
	{
		return window_.front();
	}

	template <typename Predicate>
	inline bool Lexer::TryPred(Predicate pred)
	{
		if (!window_.empty() && pred(Peek()))
		{
			Consume();
			return true;
		}

		return false;
	}

	inline bool Lexer::Try(char ch)
	{
		if (!window_.empty() && Peek() == ch)
		{
			Consume();
			return true;
		}

		return false;
	}

	inline bool Lexer::TryAny(std::string_view any)
	{
		for (auto ch : any)
		{
			if (any.find(Peek()) != any.length())
			{
				Consume();
				return true;
			}
		}

		return false;
	}

	inline bool Lexer::TrySeq(std::string_view seq)
	{
		if (window_.compare(0, seq.length(), seq) == 0)
		{
			for (auto i = 0; i < seq.length(); ++i)
				Consume();

			return true;
		}
		else
		{
			return false;
		}
	}
}