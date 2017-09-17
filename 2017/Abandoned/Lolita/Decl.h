#pragma once
#include "AstObject.h"
#include "Type.h"
#include "Literal.h"
#include "Stmt.h"
#include <variant>
#include <vector>

using namespace std;

namespace lolita
{
	class DeclVisitor;

	class DeclBase : public AstObject
	{
	public:
		virtual void Accept(DeclVisitor&) = 0;
	};

	// decl and def of variable as well as decl of function may be grouped
	struct GroupDecl : public DeclBase
	{
		std::vector<DeclBase*> Group;

		void Accept(DeclVisitor&) override;
	};

	struct VariableDecl : public DeclBase
	{
		// FIXME: add _Thread_local support
		StorageSpecifier Spec;
		QualType Type;
		std::string Name;
		ExprBase* Init;

		void Accept(DeclVisitor&) override;
	};

	struct FunctionDecl : public DeclBase
	{
		FunctionSpecifier Spec;
		CType* Type;
		std::string Name;
		StmtBase* Body;

		void Accept(DeclVisitor&) override;
	};
}