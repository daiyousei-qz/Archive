#include "Parser.h"
#include "ParserImpl.h"
#include "AstManip.h"
#include <stack>
#include <unordered_map>
#include <optional>

using namespace std;

namespace lolita
{
	ParseTree ParseTranslationUnit(const TokenVec& src)
	{
		TokenSource reader{ src };
		reader.Try(TokenTag::LBrace);
		ParserImpl parser{reader};

		auto result = parser.ParseTranslationUnit();
		for (auto decl : result)
		{
			Print(decl);
		}

		auto& types = GetTypeTable();
		system("pause");
		throw 0;
	}
}