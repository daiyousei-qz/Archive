#include "MacroEngine.h"
#include "Lexer.h"
#include <cassert>
#include <algorithm>
#include <iterator>

using namespace std;

namespace lolita
{
	void MacroExpansionAssert(bool cond)
	{
		if (!cond)
		{
			// FIXME: throw 0?
			throw 0;
		}
	}

	MacroIR	CreateIR(const Token& tok)
	{
		return MacroIR{ tok, {} };
	}

	MacroIR CreatePlacemarker()
	{
		return {};
	}

	// test if a token is a parameter
	const MacroIRList* LookupArgs(const MacroArgsMap& args, const MacroIR& tok)
	{
		// a quick path
		// non-identifier cannot be a parameter name
		if (tok.Tok.Tag != TokenTag::Identifier)
			return nullptr;

		// lookup parameter name in args map
		for (const auto& arg_pair : args)
		{
			if (arg_pair.first == tok.Tok.Content)
				return &arg_pair.second;
		}

		return nullptr;
	}


	// Stringize a sequence of tokens
	MacroIR Stringize(const MacroIRList &ts)
	{
		string buffer;

		buffer.push_back('"');
		for (const auto &tok : ts)
		{
			if (tok.Tok.SucceedingSpace)
				buffer.push_back(' ');

			switch (tok.Tok.Tag)
			{
				// for potential tokens may contain backslash or backquote
				// iterate to escape those special characters
			case TokenTag::InvalidChar:
			case TokenTag::CharConst:
			case TokenTag::StringLiteral:
				for (char ch : tok.Tok.Content)
				{
					if (ch == '\\' || ch == '"')
						buffer.push_back('\\');

					buffer.push_back(ch);
				}
				break;

				// for destructive tokens, throw
			case TokenTag::BadHeaderName:
				MacroExpansionAssert(false);
				break;

			default:
				buffer.append(tok.Tok.Content);
				break;
			}
		}
		buffer.push_back('"');

		return MacroIR
		{
			Token{ TokenTag::StringLiteral, move(buffer), ts.front().Tok.Location, false, false },
			{ }
		};
	}

	MacroHideSet IntersectHideSet(const MacroHideSet& lhs, const MacroHideSet& rhs)
	{
		auto result = lhs;
		for (const auto& name : rhs)
		{
			// name in rhs but not in lhs
			if (find(lhs.begin(), lhs.end(), name) == lhs.end())
			{
				// append it to result
				result.emplace_back(name);
			}
		}

		return result;
	}

	// Glue two tokens together, and mark its tag unknown
	MacroIR Glue(const MacroIR &lhs, const MacroIR &rhs)
	{
		auto hide_set = IntersectHideSet(lhs.HideSet, rhs.HideSet);

		// glue two tokens
		if (lhs.Tok.Tag == TokenTag::Placemarker 
			&& rhs.Tok.Tag == TokenTag::Placemarker)
		{
			return MacroIR{ lhs.Tok, {} };
		}
		else
		{
			string buffer = lhs.Tok.Content + rhs.Tok.Content;
			auto lexer = Lexer{ buffer, nullptr };
			auto result = lexer.Next();

			// verify concatenation forms a single unique valid Token
			MacroExpansionAssert(lexer.Exhausted());
			switch (result.Tag)
			{
			case TokenTag::EndOfFile:
			case TokenTag::BadHeaderName:
				MacroExpansionAssert(false);
				break;
			}

			// rewrite location
			result.Location = lhs.Tok.Location;

			return MacroIR{ move(result), hide_set };
		}
	}

	// Paste tok into the end of list
	void PasteToList(MacroIRList &list, const MacroIR& tok)
	{
		assert(!list.empty());

		// paste
		auto glued = Glue(list.back(), tok);
		// replace the last token in list
		list.back() = move(glued);
	}

	bool TestMayExpand(const MacroIR& tok)
	{
		if (tok.Tok.Tag != TokenTag::Identifier)
			return false;

		const auto& hideset = tok.HideSet;
		return find(hideset.begin(), hideset.end(), tok.Tok.Content) == hideset.end();
	}

	const MacroDescription* LookupMacro(const MacroArchive& macros, const std::string& name)
	{
		auto iter = macros.find(name);
		return iter != macros.end() ? &iter->second : nullptr;
	}

	MacroIRList CreateReplacementList(const MacroDescription& m)
	{
		auto result = MacroIRList{};
		transform(m.Expansion.begin(), m.Expansion.end(), back_inserter(result),
			[](const Token& tok)
		{
			return CreateIR(tok);
		});

		return result;
	}

	// generate substituted form of a macro
	MacroIRList Substitute(
		const MacroArchive& macros, 
		const MacroDescription& m, 
		const MacroHideSet& hideset, 
		const MacroArgsMap& args);
	// parse arguments from sequence of tokens
	// NOTE this function assume is starts with a '('
	MacroArgsMap LoadMacroArgs(MacroIRList& is, const MacroDescription& macro);
	bool TryExpandFirstAsBuiltin(const MacroArchive& macros, MacroIRList& is);
	bool TryExpandFirstAsCustom(const MacroArchive& macros, MacroIRList& is);
	bool TryExpandFirst(const MacroArchive& macros, MacroIRList& is);
	MacroIRList Expand(const MacroArchive& macros, MacroIRList is);

	// generate a token sequence from a macro expansion
	MacroIRList Substitute(
		const MacroArchive& macros, 
		const MacroDescription& m, 
		const MacroHideSet& hideset, 
		const MacroArgsMap& args)
	{
		// internal automaton for iteration
		enum class SubstitutionState
		{
			None,
			Sharp,
			SharpSharp,
		};

		auto state = SubstitutionState::None;

		// iterate to substitute
		auto is = CreateReplacementList(m);
		auto os = MacroIRList{};
		while (!is.empty())
		{
			auto tok = move(is.front());
			is.pop_front();

			if (state == SubstitutionState::Sharp)
			{
				// state that is ready for stringification
				// expect a parameter
				if (auto actual_arg = LookupArgs(args, tok))
				{
					os.emplace_back(Stringize(*actual_arg));

					state = SubstitutionState::None;
				}
				else
				{
					// the next token after '#' must be a parameter
					MacroExpansionAssert(false);
				}
			}
			else if (state == SubstitutionState::SharpSharp)
			{
				if (auto actual_arg = LookupArgs(args, tok))
				{
					// paste first token from the actual arguments
					PasteToList(os, actual_arg->front());
					// and copy the rest
					auto succeeding_begin = next(actual_arg->begin());
					copy(succeeding_begin, actual_arg->end(), back_inserter(os));
				}
				else
				{
					PasteToList(os, tok);
				}

				state = SubstitutionState::None;
			}
			else // SubstitutionState::None
			{
				// try to transfer state
				if (tok.Tok.Tag == TokenTag::Sharp)
				{
					state = SubstitutionState::Sharp;
				}
				else if (tok.Tok.Tag == TokenTag::SharpSharp)
				{
					// leading or tailing "##" is not allowed
					// FIXME: reason of failure not specified
					MacroExpansionAssert(!os.empty() && !is.empty());

					state = SubstitutionState::SharpSharp;
				}
				else
				{
					state = SubstitutionState::None;

					if (auto actual_arg = LookupArgs(args, tok))
					{
						// for arguments, expand recursively
						auto expanded_args = Expand(macros, *actual_arg);
						move(expanded_args.begin(), expanded_args.end(), back_inserter(os));
					}
					else
					{
						// for trivial tokens, simply echo
						os.emplace_back(move(tok));
					}
				}
			}
		}

		// this may fail if last valid token is # without pasting
		MacroExpansionAssert(state == SubstitutionState::None);

		// update hide set for each token generated
		for (auto &tok : os)
		{
			tok.HideSet = hideset;
		}

		return os;
	}

	MacroArgsMap LoadMacroArgs(MacroIRList& is, const MacroDescription& macro)
	{
		assert(!is.empty());
		assert(is.front().Tok.Tag == TokenTag::LParenthesis);
		assert(!is.front().Tok.SucceedingSpace);

		size_t nest_depth = 1;

		MacroArgsMap result;
		MacroIRList args_buffer;

		is.pop_front();
		while (true)
		{
			// no coresponding ')' can be found
			// FIXME: reason of failure not specified
			MacroExpansionAssert(!is.empty());

			const auto &first = is.front();
			if (nest_depth == 1 &&
				first.Tok.Tag == TokenTag::Comma &&
				result.size() < macro.Params.size())
			{
				// assuming Params is alway correct (without duplicates)
				is.pop_front();
				result.emplace_back(make_pair(macro.Params[result.size()], move(args_buffer)));
				continue;

				// NOTE that all extra arguments are put in __VA_ARGS__
				// even if the macro doesn't report it
				// in that case, an error would be reported later
			}
			else if (first.Tok.Tag == TokenTag::LParenthesis)
			{
				nest_depth += 1;
			}
			else if (first.Tok.Tag == TokenTag::RParenthesis)
			{
				nest_depth -= 1;

				if (nest_depth == 0)
				{
					// remove closing ')'
					is.pop_front();

					// finalize parsing
					string param_name = result.size() < macro.Params.size()
						? macro.Params[result.size()]
						: "__VA_ARGS__";

					result.emplace_back(make_pair(macro.Params[result.size()], move(args_buffer)));

					// exit parsing
					break;
				}
			}

			args_buffer.emplace_back(move(is.front()));
			is.pop_front();
		}

		// verify there's enough args loaded
		if (result.size() < macro.Params.size())
		{
			// FIXME: throw 0?
			throw 0;
		}

		// trim arguments and replace empty one with placemarker
		for (auto &item : result)
		{
			MacroIRList &args = item.second;

			if (args.empty())
			{
				// append a placemarker if empty
				args.emplace_back(CreatePlacemarker());
			}
			else
			{
				// discard leading and tailing whitespace
				args.front().Tok.SucceedingSpace = false;
			}
		}

		return result;
	}

	bool TryExpandFirstAsBuiltin(const MacroArchive& macros, MacroIRList& is)
	{
		auto& first_tok = is.front().Tok;
		if (first_tok.Content == "__FILE__")
		{
			first_tok.Tag = TokenTag::StringLiteral;
			first_tok.Content = first_tok.Location.File->Url();
		}
		else if (first_tok.Content == "__LINE__")
		{
			first_tok.Tag = TokenTag::IntegerConst;
			first_tok.Content = std::to_string(first_tok.Location.Line);
		}
		else
		{
			return false;
		}

		return true;
	}

	bool TryExpandFirstAsCustom(const MacroArchive& macros, MacroIRList& is)
	{
		if (auto pmacro = LookupMacro(macros, is.front().Tok.Content))
		{
			// still have a chance not to be expanded
			// NOTE function-like macro without strictedly followed '('
			if (!pmacro->ObjectLike) // i.e. function-liken
			{
				if (is.size() == 1)
					return false;

				const auto& second = is[1];
				if (second.Tok.Tag != TokenTag::LParenthesis
					|| second.Tok.SucceedingSpace)
				{
					return false;
				}
			}
			
			// load hide set
			auto hideset = is.front().HideSet;
			is.pop_front();
			// load args
			auto args_map = pmacro->ObjectLike
				? MacroArgsMap{}
				: LoadMacroArgs(is, *pmacro);

			auto subst = Substitute(macros, *pmacro, hideset, args_map);
			move(subst.rbegin(), subst.rend(), front_inserter(is));
		}
	}

	// try to expand first token in $is as a macro
	bool TryExpandFirst(const MacroArchive& macros, MacroIRList& is)
	{
		assert(!is.empty());
		
		// a quick path
		if (!TestMayExpand(is.front()))
			return false;

		return TryExpandFirstAsBuiltin(macros, is)
			|| TryExpandFirstAsCustom(macros, is);
	}

	// expand macros in $is and yields result
	MacroIRList Expand(const MacroArchive& macros, MacroIRList is)
	{
		MacroIRList os;
		while (!is.empty())
		{
			if (!TryExpandFirst(macros, is))
			{
				// if first token cannot expand as a macro, echo
				os.emplace_back(move(is.front()));
				is.pop_front();
			}
		}

		return os;
	}

	// Impl
	//

	std::vector<Token> ExpandMacro(TokenSource& src, const MacroArchive& macros, bool obj_like)
	{
		assert(src.Test(TokenTag::Identifier));
		assert(macros.find(src.Peek().Content) != macros.end());
		auto is = MacroIRList{ CreateIR(src.Consume()) };
		if (!obj_like)
		{
			assert(src.Test(TokenTag::LParenthesis));
			assert(!src.Peek().SucceedingSpace);
			auto depth = 0;

			// load arguments
			while (true)
			{
				// FIXME: throw 0?
				if (src.Exhausted())
					throw 0;

				if (src.Test(TokenTag::LParenthesis))
					depth += 1;
				else if (src.Test(TokenTag::RParenthesis))
					depth -= 1;

				is.emplace_back(CreateIR(src.Consume()));

				if (depth == 0)
				{
					break;
				}
			}
		}

		auto os = Expand(macros, is);
		auto result = vector<Token>{};
		for (auto& ir : os)
		{
			if (ir.Tok.Tag != TokenTag::Placemarker)
			{
				result.emplace_back(move(ir.Tok));
			}
		}

		return result;
	}

}