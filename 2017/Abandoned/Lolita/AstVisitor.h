#pragma once
#include "Expr.h"
#include "Stmt.h"
#include "Decl.h"

namespace lolita
{
	// Visitor
	//

	class ExprVisitor
	{
	public:
		virtual void VisitLiteralExpr(LiteralExpr&) { }
		virtual void VisitVariableExpr(VariableExpr&) { }
		virtual void VisitCastExpr(CastExpr&) { }
		virtual void VisitUnaryExpr(UnaryExpr&) { }
		virtual void VisitBinaryExpr(BinaryExpr&) { }
		virtual void VisitConditionalExpr(ConditionalExpr&) { }
		virtual void VisitCommaExpr(CommaExpr&) { }
		virtual void VisitAccessExpr(AccessExpr&) { }
		virtual void VisitSubscriptExpr(SubscriptExpr&) { }
		virtual void VisitInvokeExpr(InvokeExpr&) { }
		virtual void VisitTypeMetaExpr(TypeMetaExpr&) { }
		virtual void VisitValueMetaExpr(ValueMetaExpr&) { }
	};

	class StmtVisitor
	{
	public:
		virtual void VisitDeclStmt(DeclStmt&) { }
		virtual void VisitCompoundStmt(CompoundStmt&) { }
		virtual void VisitExprStmt(ExprStmt&) { }
		virtual void VisitIfStmt(IfStmt&) { }
		virtual void VisitForStmt(ForStmt&) { }
		virtual void VisitWhileStmt(WhileStmt&) { }
		virtual void VisitJumpStmt(JumpStmt&) { }
		virtual void VisitReturnStmt(ReturnStmt&) { }
	};

	class DeclVisitor
	{
	public:
		virtual void VisitGroupDecl(GroupDecl&) { }
		virtual void VisitVariableDecl(VariableDecl&) { }
		virtual void VisitFunctionDecl(FunctionDecl&) { }
	};
}