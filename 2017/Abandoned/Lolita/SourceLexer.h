#pragma once
#include "Token.h"
#include "SourceFile.h"

namespace lolita
{
	TokenVec LexSourceFile(const SourceFile* file);
}