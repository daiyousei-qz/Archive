#pragma once
#include "AstArena.h"
#include "Scope.h"
#include "SourceLocation.h"

namespace lolita
{
	// This class provide Actions during syntax directed translation
	class AstBuilder
	{
	public:
		// Utility Functions
		//
		void EnterFunction();
		void ExitFunction();

		void AnnotateLocation(SourceLocation loc);

		void ReportError(const std::string& msg);

		// Decl Factory
		//

		// Expr Factory
		//

		// NOTE those factory funtions does not map to a unique Ast node type
		ExprBase* NewLiteralExpr(const CConstant& val);
		ExprBase* NewVariableExpr(const std::string& name);
		ExprBase* NewCastExpr(CType* type, ExprBase* val);
		ExprBase* NewUnaryExpr(UnaryOp op, ExprBase* val);
		ExprBase* NewBinaryExpr(BinaryOp op, ExprBase* lhs, ExprBase* rhs);
		ExprBase* NewConditionalExpr(ExprBase* cond, ExprBase* yes, ExprBase* no);
		ExprBase* NewCommaExpr(const std::vector<ExprBase*>& list);
		ExprBase* NewAccessExpr(ExprBase* obj, const std::string& name, bool deref);
		ExprBase* NewInvokeExpr(ExprBase* callee, std::vector<ExprBase*> args);
		ExprBase* NewSubscriptExpr(ExprBase* data, ExprBase* index);

		ExprBase* NewSizeOfExpr(CType* type);
		ExprBase* NewAlignOfExpr(CType* type);

		// Stmt Factory
		//
		StmtBase* NewDeclStmt(QualType type, const std::string& name, ExprBase* value);
		StmtBase* NewCompoundStmt(std::vector<StmtBase*> stmt);
		StmtBase* NewExprStmt(ExprBase* expr);
		StmtBase* NewIfStmt(ExprBase* cond, StmtBase* yes, StmtBase* no);
		// StmtBase* NewForStmt(StmtBase* init, ExprBase* cond, ExprBase* next, StmtBase* body);
		StmtBase* NewWhileStmt(ExprBase* cond, StmtBase* body);
		StmtBase* NewBreakStmt();
		StmtBase* NewContinueStmt();
		StmtBase* NewReturnStmt(ExprBase* expr);
		StmtBase* NewGotoStmt(const std::string& label);

	private:
		AstArena::Ptr arena_;

		Scope* file_scope_;
		Scope* local_scope_;
	};
}