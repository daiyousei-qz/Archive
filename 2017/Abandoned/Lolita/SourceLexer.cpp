#include "SourceLexer.h"
#include "Lexer.h"
#include <cassert>

using namespace std;

namespace lolita
{
	TokenVec LexSourceFile(const SourceFile* file)
	{
		assert(file != nullptr);

		// construct lexer
		auto lexer = Lexer{ file->Data(), file };
		
		// make state automaton
		auto pp_state = false;

		// iterate to lex the source file
		auto result = vector<Token>{};
		do
		{
			result.emplace_back(lexer.Next());

			const auto& last_tok = result.back();

			if (pp_state && !last_tok.StartOfLine)
			{
				if (last_tok.Tag == TokenTag::Identifier
					&& last_tok.Content == "include")
				{
					lexer.HintHeaderName();
				}
			}

			pp_state = last_tok.StartOfLine && last_tok.Tag == TokenTag::Sharp;

		} while (result.back().Tag != TokenTag::EndOfFile);

		return result;
	}
}