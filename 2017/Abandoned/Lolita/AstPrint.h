#pragma once
#include "AstVisitor.h"

namespace lolita
{
	void PrintAst(ExprBase* expr);
	void PrintAst(StmtBase* stmt);
	void PrintAst(DeclBase* decl);
}