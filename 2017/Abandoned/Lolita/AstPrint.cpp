#include "AstPrint.h"
#include "EnumMetadata.h"
#include "TextUtils.h"

using namespace std;

namespace lolita
{
	static int depth = 0;

	static void PrepareIdent()
	{
		for (int i = 0; i < depth; ++i)
			printf(" ");
	}

	void PrintAst(ExprBase* expr)
	{
		struct Visitor : ExprVisitor
		{
			void VisitLiteralExpr(LiteralExpr &expr) override
			{
				if (auto pi = get_if<CInteger>(&expr.Value))
				{
					printf("Integer Const: %llu\n", pi->Value());
				}
				else if (auto pf = get_if<CFloat>(&expr.Value))
				{
					printf("Float Const: %lf\n", pf->Value());
				}
				else if (auto ps = get_if<CString>(&expr.Value))
				{
					printf("String Literal: %s\n", ('"' + ps->Value() + '"').c_str());
				}
				else
				{
					throw 0;
				}
			};
			void VisitVariableExpr(VariableExpr& expr) override
			{
				printf("Variable Expr: %s\n", expr.Name.c_str());
			};
			void VisitCastExpr(CastExpr &) override { };
			void VisitUnaryExpr(UnaryExpr &) override { };
			void VisitBinaryExpr(BinaryExpr& expr)  override
			{
				printf("Binary Expr: %s\n", ToString(expr.Op));

				depth += 2;
				PrintAst(expr.Left);
				PrintAst(expr.Right);
				depth -= 2;
			};
			void VisitConditionalExpr(ConditionalExpr &) override { };
			void VisitCommaExpr(CommaExpr &) override { };
			void VisitAccessExpr(AccessExpr &) override { };
			void VisitSubscriptExpr(SubscriptExpr& expr) override
			{
				printf("Subscript Expr:\n");

				depth += 2;
				PrintAst(expr.object_);
				PrintAst(expr.index_);
				depth -= 2;
			}
			void VisitInvokeExpr(InvokeExpr& expr) override
			{
				printf("Invoke Expr:\n");

				depth += 2;
				PrintAst(expr.callee_);
				for (auto child : expr.args_)
				{
					PrintAst(child);
				}
				depth -= 2;
			};
			void VisitTypeMetaExpr(TypeMetaExpr &) override { };
			void VisitValueMetaExpr(ValueMetaExpr &) override { };
		};

		auto visitor = Visitor{};
		PrepareIdent();
		if (expr != nullptr)
		{
			expr->Accept(visitor);
		}
		else
		{
			printf("EmptyStmt\n");
		}
	}

	void PrintAst(StmtBase* stmt)
	{
		struct Visitor : StmtVisitor
		{
			void VisitDeclStmt(DeclStmt& stmt) override
			{
				printf("Decl Stmt: %s\n", stmt.Name.c_str());
			};
			void VisitCompoundStmt(CompoundStmt& stmt) override
			{
				printf("Compound Stmt:\n");

				depth += 2;
				for (auto child : stmt.Children)
				{
					PrintAst(child);
				}
				depth -= 2;
			};
			void VisitExprStmt(ExprStmt& stmt) override
			{
				printf("Expr Stmt:\n");

				depth += 2;
				PrintAst(stmt.Expr);
				depth -= 2;
			};
			void VisitIfStmt(IfStmt& stmt) override
			{
				printf("If Stmt:\n");

				depth += 2;
				PrintAst(stmt.Choice);
				PrintAst(stmt.First);
				PrintAst(stmt.Second);
				depth -= 2;
			};
			void VisitForStmt(ForStmt&) override { };
			void VisitWhileStmt(WhileStmt& stmt) override
			{
				printf("While Stmt:\n");

				depth += 2;
				PrintAst(stmt.Predicate);
				PrintAst(stmt.Body);
				depth -= 2;
			};
			void VisitJumpStmt(JumpStmt&) override { };
			void VisitReturnStmt(ReturnStmt& stmt) override
			{
				printf("Return Stmt:\n");

				if (stmt.Expr)
				{
					depth += 2;
					PrintAst(stmt.Expr);
					depth -= 2;
				}
			};
		};

		auto visitor = Visitor{};
		PrepareIdent();
		stmt->Accept(visitor);
	}

	void PrintAst(DeclBase* decl)
	{
		struct Visitor : DeclVisitor
		{
			void VisitGroupDecl(GroupDecl& decl) override { }
			void VisitVariableDecl(VariableDecl& decl) override { }
			void VisitFunctionDecl(FunctionDecl& decl) override
			{
				printf("Function: %s\n", decl.Name.c_str());

				depth += 2;
				PrintAst(decl.Body);
				depth -= 2;
			}
		};

		auto visitor = Visitor{};
		PrepareIdent();
		decl->Accept(visitor);
	}
}