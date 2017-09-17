#pragma once
#include "Literal.h"
#include "Token.h"
#include <string>

namespace lolita
{
	CInteger	ParseIntegerConst(const Token& tok);
	CFloat		ParseFloatConst(const Token& tok);
	CInteger	ParseCharConst(const Token& tok);
	CString		ParseStringLiteral(const Token& tok);
}