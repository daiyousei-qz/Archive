#include "AstBuilder.h"
#include "TextUtils.h"

namespace lolita
{
	// Expr Factory
	//
	ExprBase* AstBuilder::NewLiteralExpr(const CConstant& value)
	{
		return arena_->MakeAstObject<LiteralExpr>(value);
	}
	ExprBase* AstBuilder::NewVariableExpr(const std::string& name)
	{
		// ensure name refers to a variable
		auto entity = local_scope_->LookupEntity(name);
		if (entity == nullptr || entity->StorageClass != StorageSpecifier::Typedef)
		{
			ReportError(FormatString("identifier %s does not refer to a variable", name.c_str()));
		}

		return arena_->MakeAstObject<VariableExpr>(entity);
	}
	ExprBase* AstBuilder::NewCastExpr(CType* target_type, ExprBase* val)
	{
		// 6.5.4 Cast operators

		// void target type discards expression value
		if (target_type->IsVoid())
		{
			return arena_->MakeAstObject<CastExpr>(target_type, val);
		}

		CType* src_type = val->GetType();
		if (!src_type->IsScalar())
		{
			throw "cannot cast a non-scalar type";
		}

		// target_type must be either scalar type or void
		if (target_type->IsPointer())
		{
			if (src_type->IsFloat())
				throw "cannot cast a float to a pointer";

			// FIXME: should I ensure correct alignment?
		}
		else if (target_type->IsInteger())
		{
			// do nothing
		}
		else if (target_type->IsFloat())
		{
			if (src_type->IsPointer())
				throw "cannot cast a pointer to a float";
		}
		else
		{
			throw "casting requires scalar type";
		}

		return arena_->MakeAstObject<CastExpr>(target_type, val);
	}
	ExprBase* AstBuilder::NewUnaryExpr(UnaryOp op, ExprBase* expr)
	{
		// FIXME: ensure op(expr) can apply to expr

		return arena_->MakeAstObject<UnaryExpr>(op, expr);
	}
	ExprBase* AstBuilder::NewBinaryExpr(BinaryOp op, ExprBase* lhs, ExprBase* rhs)
	{
		// FIXME: ensure op(lhs, rhs) is valid

		return arena_->MakeAstObject<BinaryExpr>(op, lhs, rhs);
	}
	ExprBase* AstBuilder::NewConditionalExpr(ExprBase* cond, ExprBase* yes, ExprBase* no)
	{
		// 6.5.15 Conditional operator

		if (!cond->GetType()->IsScalar())
		{
			throw "condition expression must have scalar type";
		}

		// FIXME: ensure yes and no have the same type

		return arena_->MakeAstObject<ConditionalExpr>(cond, yes, no);
	}
	ExprBase* AstBuilder::NewCommaExpr(const std::vector<ExprBase*>& list)
	{
		return arena_->MakeAstObject<CommaExpr>(list);
	}
	ExprBase* AstBuilder::NewAccessExpr(ExprBase* obj, const std::string& name, bool deref)
	{
		auto actual_obj = deref
			? NewUnaryExpr(UnaryOp::Deref, obj)
			: obj;

		// FIXME: ensure actual_obj has member with name specified

		return arena_->MakeAstObject<AccessExpr>(actual_obj, name);
	}
	ExprBase* AstBuilder::NewInvokeExpr(ExprBase* callee, std::vector<ExprBase*> args)
	{
		// FIXME:
		auto type = dynamic_cast<PointerType*>(callee->GetType());
		if (type == nullptr || !type->BaseType().Type->IsFunction())
		{
			throw "only function or pointer to function should be called";
		}

		// FIXME: ensure correct number of arguments are provided

		return arena_->MakeAstObject<InvokeExpr>(callee, args);
	}
	ExprBase* AstBuilder::NewSubscriptExpr(ExprBase* data, ExprBase* index)
	{
		// FIXME: ensure subscript applies to data (array or pointer)
		// FIXME: ensure index is an integer

		return arena_->MakeAstObject<SubscriptExpr>(data, index);
	}

	// Stmt Factory
	//
	StmtBase* AstBuilder::NewDeclStmt(QualType type, const std::string& name, ExprBase* value)
	{
		auto result = arena_->MakeAstObject<DeclStmt>();
		result->Type = type;
		result->Name = name;
		result->Value = value;

		return result;
	}
	StmtBase* AstBuilder::NewCompoundStmt(std::vector<StmtBase*> children)
	{
		auto result = arena_->MakeAstObject<CompoundStmt>();
		result->Children = std::move(children);

		return result;
	}
	StmtBase* AstBuilder::NewExprStmt(ExprBase* expr)
	{
		auto result = arena_->MakeAstObject<ExprStmt>();
		result->Expr = expr;

		return result;
	}
	StmtBase* AstBuilder::NewIfStmt(ExprBase* cond, StmtBase* yes, StmtBase* no)
	{
		auto result = arena_->MakeAstObject<IfStmt>();
		result->Choice = cond;
		result->First = yes;
		result->Second = no;

		return result;
	}
	StmtBase* AstBuilder::NewWhileStmt(ExprBase* cond, StmtBase* body)
	{
		auto result = arena_->MakeAstObject<WhileStmt>();
		result->Predicate = cond;
		result->Body = body;

		return result;
	}
	StmtBase* AstBuilder::NewBreakStmt()
	{
		auto result = arena_->MakeAstObject<JumpStmt>();
		result->Strategy = JumpStrategy::Break;

		return result;
	}
	StmtBase* AstBuilder::NewContinueStmt()
	{
		auto result = arena_->MakeAstObject<JumpStmt>();
		result->Strategy = JumpStrategy::Continue;

		return result;
	}
	StmtBase* AstBuilder::NewReturnStmt(ExprBase* expr)
	{
		auto result = arena_->MakeAstObject<ReturnStmt>();
		result->Expr = expr;

		return result;
	}
	StmtBase* AstBuilder::NewGotoStmt(const std::string& label)
	{
		auto result = arena_->MakeAstObject<JumpStmt>();
		result->Strategy = JumpStrategy::Goto;
		result->TargetLabel = label;

		return result;
	}
}