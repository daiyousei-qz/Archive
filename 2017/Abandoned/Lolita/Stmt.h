#pragma once
#include "Basic.h"
#include "AstObject.h"
#include "Expr.h"
#include "Scope.h"
#include <vector>
#include <string>

namespace lolita
{
	class StmtVisitor;

	class StmtBase : public AstObject
	{
	public:
		virtual void Accept(StmtVisitor&) = 0;

	public:
		// override the innermost scope
		// maybe nullptr, where no scope should be added
		Scope* LocalScope;

		StringVec Labels;
	};

	// Stmt
	//

	struct DeclStmt : public StmtBase
	{
		QualType Type;
		std::string Name;
		ExprBase* Value;

		void Accept(StmtVisitor&) override;
	};

	struct CompoundStmt : public StmtBase
	{
		std::vector<StmtBase*> Children;

		void Accept(StmtVisitor&) override;
	};

	struct ExprStmt :public StmtBase
	{
		ExprBase* Expr;

		void Accept(StmtVisitor&) override;
	};

	struct IfStmt : public StmtBase
	{
		ExprBase* Choice;
		StmtBase* First;
		StmtBase* Second;

		void Accept(StmtVisitor&) override;
	};

	struct ForStmt : public StmtBase
	{
		StmtBase *Child;

		void Accept(StmtVisitor&) override;
	};

	struct WhileStmt : public StmtBase
	{
		ExprBase* Predicate;
		StmtBase* Body;

		void Accept(StmtVisitor&) override;
	};

	struct JumpStmt : public StmtBase
	{
		JumpStrategy Strategy;
		std::string TargetLabel;

		void Accept(StmtVisitor&) override;
	};

	struct ReturnStmt : public StmtBase
	{
		ExprBase* Expr;

		void Accept(StmtVisitor&) override;
	};
}