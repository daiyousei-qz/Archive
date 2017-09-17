#include "AstVisitor.h"

namespace lolita
{
	// Accept Function for ExprBase
	//
	void LiteralExpr::Accept(ExprVisitor& visitor) { visitor.VisitLiteralExpr(*this); }
	void VariableExpr::Accept(ExprVisitor& visitor) { visitor.VisitVariableExpr(*this); }
	void CastExpr::Accept(ExprVisitor& visitor) { visitor.VisitCastExpr(*this); }
	void UnaryExpr::Accept(ExprVisitor& visitor) { visitor.VisitUnaryExpr(*this); }
	void BinaryExpr::Accept(ExprVisitor& visitor) { visitor.VisitBinaryExpr(*this); }
	void ConditionalExpr::Accept(ExprVisitor& visitor) { visitor.VisitConditionalExpr(*this); }
	void CommaExpr::Accept(ExprVisitor& visitor) { visitor.VisitCommaExpr(*this); }
	void AccessExpr::Accept(ExprVisitor& visitor) { visitor.VisitAccessExpr(*this); }
	void SubscriptExpr::Accept(ExprVisitor& visitor) { visitor.VisitSubscriptExpr(*this); }
	void InvokeExpr::Accept(ExprVisitor& visitor) { visitor.VisitInvokeExpr(*this); }
	void TypeMetaExpr::Accept(ExprVisitor& visitor) { visitor.VisitTypeMetaExpr(*this); }
	void ValueMetaExpr::Accept(ExprVisitor& visitor) { visitor.VisitValueMetaExpr(*this); }

	// Accept Function for StmtBase
	//
	void DeclStmt::Accept(StmtVisitor& visitor) { visitor.VisitDeclStmt(*this); }
	void CompoundStmt::Accept(StmtVisitor& visitor) { visitor.VisitCompoundStmt(*this); }
	void ExprStmt::Accept(StmtVisitor& visitor) { visitor.VisitExprStmt(*this); }
	void IfStmt::Accept(StmtVisitor& visitor) { visitor.VisitIfStmt(*this); }
	void ForStmt::Accept(StmtVisitor& visitor) { visitor.VisitForStmt(*this); }
	void WhileStmt::Accept(StmtVisitor& visitor) { visitor.VisitWhileStmt(*this); }
	void JumpStmt::Accept(StmtVisitor& visitor) { visitor.VisitJumpStmt(*this); }
	void ReturnStmt::Accept(StmtVisitor& visitor) { visitor.VisitReturnStmt(*this); }

	// Accept Function for DeclBase
	//
	void GroupDecl::Accept(DeclVisitor& visitor) { visitor.VisitGroupDecl(*this); }
	void VariableDecl::Accept(DeclVisitor& visitor) { visitor.VisitVariableDecl(*this); }
	void FunctionDecl::Accept(DeclVisitor& visitor) { visitor.VisitFunctionDecl(*this); }
}