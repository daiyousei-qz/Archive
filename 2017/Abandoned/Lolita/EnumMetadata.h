#pragma once
#include "Token.h"
#include "AstModel.h"
#include <string>

namespace lolita
{
	const char* ToString(TokenTag tag);
	const char* ToString(UnaryOp op);
	const char* ToString(BinaryOp op);
}