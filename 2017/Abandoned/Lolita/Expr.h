#pragma once
#include "AstObject.h"
#include "Literal.h"

namespace lolita
{
	struct NamedEntity;

	class ExprVisitor;

	class ExprBase : public AstObject
	{
	public:
		virtual void Accept(ExprVisitor&) = 0;

		virtual bool IsConstExpr() const { return false; }
		virtual bool IsAssignable() const { return false; }

		CType* GetType() const { return type_; }

	protected:
		CType* type_ = nullptr;
	};

	// Expr
	//

	class LiteralExpr : public ExprBase
	{
	public:
		LiteralExpr(const CConstant& value)
			: value_(value) 
		{
			// type_ = ...
		}

		const auto& Value() const { return value_; }

		void Accept(ExprVisitor&) override;
		bool IsConstExpr() const override { return true; }

	private:
		CConstant value_;
	};

	class VariableExpr : public ExprBase
	{
	public:
		VariableExpr(NamedEntity* entity)
			: entity_(entity) 
		{
			assert(entity != nullptr);
		}

		auto Entity() const { return entity_; }

		void Accept(ExprVisitor&) override;
		bool IsAssignable() const override { return true; }

	private:
		NamedEntity* entity_;
	};

	class CastExpr : public ExprBase
	{
	public:
		CastExpr(CType* target_type, ExprBase* castee)
			: target_(target_type), expr_(castee)
		{
			assert(target_type != nullptr && castee != nullptr);
		}

		auto Target() const { return target_; }
		auto UnderlyingExpr() const { return expr_; }

		void Accept(ExprVisitor&) override;

	private:
		CType* target_;
		ExprBase* expr_;
	};

	class UnaryExpr : public ExprBase
	{
		UnaryExpr(UnaryOp op, ExprBase* expr)
			: op_(op), expr_(expr) 
		{
			assert(expr != nullptr);
		}

		auto Op() const { return op_; }
		auto UnderlyingExpr() const { return expr_; }

		void Accept(ExprVisitor&) override;
		bool IsConstExpr() const override
		{
			return expr_->IsConstExpr();
		}

	private:
		UnaryOp op_;
		ExprBase* expr_;
	};

	class BinaryExpr : public ExprBase
	{
	public:
		BinaryExpr(BinaryOp op, ExprBase* lhs, ExprBase* rhs)
			: op_(op), left_(lhs), right_(rhs)
		{
			assert(lhs != nullptr && rhs != nullptr);
		}

		auto Op() const { return op_; }
		auto LeftExpr() const { return left_; }
		auto RightExpr() const { return right_; }

		void Accept(ExprVisitor&) override;
		bool IsConstExpr() const override
		{
			return left_->IsConstExpr() && right_->IsConstExpr();
		}
	private:
		BinaryOp op_;
		ExprBase* left_;
		ExprBase* right_;
	};

	class ConditionalExpr : public ExprBase
	{
	public:
		ConditionalExpr(ExprBase* choice, ExprBase* first, ExprBase* second)
			: selector_(choice), first_(first), second_(second)
		{
			assert(choice != nullptr
				&& first != nullptr
				&& second != nullptr);
		}

		auto SelectorExpr() const { return selector_; }
		auto FirstExpr() const { return first_; }
		auto SecondExpr() const { return second_; }

		void Accept(ExprVisitor&) override;
		bool IsConstExpr() const override
		{
			return selector_->IsConstExpr()
				&& first_->IsConstExpr()
				&& second_->IsConstExpr();
		}

	private:
		ExprBase* selector_;
		ExprBase* first_;
		ExprBase* second_;
	};

	class CommaExpr : public ExprBase
	{
	public:
		CommaExpr(const std::vector<ExprBase*>& list)
			: seq_(list)
		{
			assert(!list.empty());
		}

		const auto& Seq() const { return seq_; }

		void Accept(ExprVisitor&) override;

	private:
		std::vector<ExprBase*> seq_;
	};

	class AccessExpr : public ExprBase
	{
	public:
		AccessExpr(ExprBase* obj, const std::string& name)
			: object_(obj), name_(name)
		{
			assert(obj != nullptr && !name.empty());
		}

		auto Object() const { return object_; }
		const auto& Name() const { return name_; }

		void Accept(ExprVisitor&) override;

	private:
		ExprBase* object_;
		std::string name_;

	};

	class InvokeExpr : public ExprBase
	{
	public:
		InvokeExpr(ExprBase* func, const std::vector<ExprBase*>& args)
			: callee_(func), args_(args)
		{
			assert(func != nullptr);
		}

		void Accept(ExprVisitor&) override;
	private:
		ExprBase *callee_;
		std::vector<ExprBase*> args_;
	};

	class SubscriptExpr : public ExprBase
	{
	public:
		SubscriptExpr(ExprBase* data, ExprBase* index)
			:object_(data), index_(index)
		{
			assert(data != nullptr && index != nullptr);
		}

		void Accept(ExprVisitor&) override;
	private:
		ExprBase* object_;
		ExprBase* index_;
	};
}