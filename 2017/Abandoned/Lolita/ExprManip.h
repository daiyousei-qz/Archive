#pragma once
#include "AstVisitor.h"
#include "Type.h"

namespace lolita
{
	QualType EvalExprType(ExprBase* expr)
	{
		struct Visitor : public ExprVisitor
		{
			 void VisitLiteralExpr(LiteralExpr& expr) override
			 {
			 
			 }
			 void VisitVariableExpr(VariableExpr& expr) override
			 {
			 
			 }
			 void VisitCastExpr(CastExpr& expr) override
			 {

			 }
			 void VisitUnaryExpr(UnaryExpr& expr) override
			 {
			 
			 }
			 void VisitBinaryExpr(BinaryExpr& expr) override
			 { 
			 
			 }
			 void VisitConditionalExpr(ConditionalExpr& expr) override
			 { 
			 
			 }
			 void VisitCommaExpr(CommaExpr& expr) override
			 {
			 
			 }
			 void VisitAccessExpr(AccessExpr& expr) override
			 { 
			 
			 }
			 void VisitSubscriptExpr(SubscriptExpr& expr) override
			 { 
			 
			 }
			 void VisitInvokeExpr(InvokeExpr& expr) override
			 { 
			 
			 }

			 QualType Result;
		};

		assert(expr != nullptr);
		auto visitor = Visitor{};
		expr->Accept(visitor);

		return visitor.Result;
	}
}