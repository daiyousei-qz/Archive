#include "EnumMetadata.h"
#include "ParserHelper.h"
#include <map>

using namespace std;

namespace lolita
{
	static constexpr TokenTag kLastTokenTag = TokenTag::StringLiteral;
	static constexpr const char* kLastTokenTagName = "<StringLiteral>";

	const char* ToString(TokenTag tag)
	{
		static const char* name_list[] =
		{
			"<EOF>",
			"<HeaderName>",
			"<Placemarker>",
			"#",
			"##",
			"<InvalidChar>",
			"BadHeaderName",

			"(",
			")",
			"[",
			"]",
			"{",
			"{",

			"=",
			"+=",
			"-=",
			"*=",
			"/=",
			"%=",
			"&=",
			"|=",
			"^=",
			"<<=",
			">>=",

			"++",
			"--",

			"+",
			"-",
			"*",
			"/",
			"%",
			"&",
			"|",
			"^",
			"~",
			"<<",
			">>",

			"!",
			"&&",
			"||",

			"==",
			"!=",
			"<",
			">",
			"<=",
			">=",

			".",
			"...",
			"->",
			"?",
			":",
			";",
			",",
			
			"_Alignas",
			"_Alignof",
			"_Atomic",
			"auto",
			"break",
			"_Bool",
			"case",
			"char",
			"_Complex",
			"const",
			"continue",
			"default",
			"do",
			"double",
			"else",
			"enum",
			"extern",
			"float",
			"for",
			"_Generic",
			"goto",
			"if",
			"_Imaginary",
			"inline",
			"int",
			"long",
			"_Noreturn",
			"register",
			"restrict",
			"return",
			"short",
			"signed",
			"sizeof",
			"static",
			"_Static_assert",
			"struct",
			"switch",
			"_Thread_local",
			"typedef",
			"union",
			"unsigned",
			"void",
			"volatile",
			"while",

			"<Identifier>",
			"<IntegerConst>",
			"<FloatConst>",
			"<CharConst>",
			"<StringLiteral>",
		};
		assert(name_list[static_cast<int>(kLastTokenTag)] == kLastTokenTagName);

		return name_list[static_cast<int>(tag)];
	}

	const char* ToString(UnaryOp op)
	{
		static const auto name_list = []
		{
			vector<const char*> result;

			const auto bound = static_cast<int>(kLastTokenTag) + 1;
			for (int i = 0; i < bound; ++i)
			{
				auto tag = static_cast<TokenTag>(i);
				auto op = TranslateUnaryOp(tag);
				if (op)
				{
					auto op_val = static_cast<int>(*op);
					if (op_val > result.size())
						result.resize(op_val + 1);

					result[op_val] = ToString(tag);
				}
			}

			return result;
		}();

		return name_list[static_cast<int>(op)];
	}

	const char* ToString(BinaryOp op)
	{
		static const auto name_list = [] 
		{
			vector<const char*> result;

			const auto bound = static_cast<int>(kLastTokenTag) + 1;
			for (int i = 0; i < bound; ++i)
			{
				auto tag = static_cast<TokenTag>(i);
				auto op = TranslateBinaryOp(tag);
				if (op)
				{
					auto op_val = static_cast<int>(*op);
					if (op_val >= result.size())
						result.resize(op_val + 1);

					result[op_val] = ToString(tag);
				}
			}

			return result;
		}();

		return name_list[static_cast<int>(op)];
	}
}