#pragma once
#include "Basic.h"
#include "Token.h"
#include <vector>
#include <deque>
#include <unordered_map>

namespace lolita
{
	struct MacroIR;
	struct MacroDescription;

	using MacroArchive = std::unordered_map<std::string, MacroDescription>;
	using MacroHideSet = std::vector<std::string>;
	using MacroIRList = std::deque<MacroIR>;
	using MacroArgument = std::pair<std::string, MacroIRList>;
	using MacroArgsMap = std::vector<MacroArgument>;

	struct MacroIR
	{
		Token Tok;

		MacroHideSet HideSet;
	};

	// A description to a custom macro
	struct MacroDescription
	{
		std::string Name;

		StringVec Params;

		bool ObjectLike;

		bool VariadicArgs;

		std::vector<Token> Expansion;
	};

	std::vector<Token> ExpandMacro(TokenSource& src, const MacroArchive& macros, bool obj_like);
}