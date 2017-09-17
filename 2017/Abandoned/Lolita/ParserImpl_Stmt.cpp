#include "ParserImpl.h"
#include <string>
#include <vector>

using namespace std;

namespace lolita
{
	// Parsing Stmt Impl
	//

	// FIXME: redo DeclStmt to make it has value of DeclBase*
	StmtBase* ParserImpl::ParseDeclStmt()
	{
		auto storage = StorageSpecifier::None;
		auto thd_local = false;
		auto base_type = ParseDeclSpec(nullptr, &storage, &thd_local, nullptr);

		// FIXME: support multiple declarators
		auto type = ParseDeclarator(base_type, nullptr);

		auto value = static_cast<ExprBase*>(nullptr);
		if (src_.Try(TokenTag::Assign))
		{
			if (src_.Try(TokenTag::LBrace))
			{
				// FIXME: support scalar initializer
				throw 0;
			}
			else
			{
				value = ParseAssignmentExpr();
			}
		}

		ExpectToken(TokenTag::Semicolon);

		// local_scope_->DeclareEntity(name, type, storage);
		return arena_.NewDeclStmt(type.first, *type.second, nullptr);
	}

	StmtBase* ParserImpl::ParseExprStmt()
	{
		auto expr = ParseExpr();
		ExpectToken(TokenTag::Semicolon);

		return arena_.NewExprStmt(expr);
	}
	StmtBase* ParserImpl::ParseIfStmt()
	{
		assert(src_.LastConsumed().Tag == TokenTag::If);

		ExpectToken(TokenTag::LParenthesis);
		auto expr = ParseExpr();
		ExpectToken(TokenTag::RParenthesis);

		auto yes_stmt = ParseStmt();

		// try optional else
		auto no_stmt = static_cast<StmtBase*>(nullptr);
		if (src_.Try(TokenTag::Else))
		{
			no_stmt = ParseStmt();
		}

		return arena_.NewIfStmt(expr, yes_stmt, no_stmt);
	}
	StmtBase* ParserImpl::ParseSwitchStmt()
	{
		assert(src_.LastConsumed().Tag == TokenTag::Switch);
		throw 0;
	}
	StmtBase* ParserImpl::ParseForStmt()
	{
		assert(src_.LastConsumed().Tag == TokenTag::For);
		throw 0;
	}
	StmtBase* ParserImpl::ParseWhileStmt()
	{
		assert(src_.LastConsumed().Tag == TokenTag::While);
		ExpectToken(TokenTag::LParenthesis);
		auto cond = ParseExpr();
		ExpectToken(TokenTag::RParenthesis);
		auto body = ParseStmt();

		return arena_.NewWhileStmt(cond, body);
	}
	StmtBase* ParserImpl::ParseDoWhileStmt()
	{
		assert(src_.LastConsumed().Tag == TokenTag::Do);
		throw 0;
	}
	StmtBase* ParserImpl::ParseCompoundStmt()
	{
		assert(src_.LastConsumed().Tag == TokenTag::LBrace);

		vector<StmtBase*> children;
		while (!src_.Try(TokenTag::RBrace))
		{
			children.push_back(ParseStmt());
		}

		return arena_.NewCompoundStmt(move(children));
	}
	StmtBase* ParserImpl::ParseUnlabelledStmt()
	{
		// parse stmt
		if (src_.Try(TokenTag::If))
		{
			return ParseIfStmt();
		}
		else if (src_.Try(TokenTag::Switch))
		{
			return ParseSwitchStmt();
		}
		else if (src_.Try(TokenTag::For))
		{
			return ParseForStmt();
		}
		else if (src_.Try(TokenTag::While))
		{
			return ParseWhileStmt();
		}
		else if (src_.Try(TokenTag::Do))
		{
			return ParseDoWhileStmt();
		}
		else if (src_.Try(TokenTag::Goto))
		{
			ExpectToken(TokenTag::Identifier);
			auto label_name = src_.LastConsumed().Content;
			ExpectToken(TokenTag::Semicolon);

			return arena_.NewGotoStmt(label_name);
		}
		else if (src_.Try(TokenTag::Break))
		{
			ExpectToken(TokenTag::Semicolon);
			return arena_.NewBreakStmt();
		}
		else if (src_.Try(TokenTag::Continue))
		{
			ExpectToken(TokenTag::Semicolon);
			return arena_.NewContinueStmt();
		}
		else if (src_.Try(TokenTag::Return))
		{
			auto expr = static_cast<ExprBase*>(nullptr);
			if (!src_.Test(TokenTag::Semicolon))
			{
				expr = ParseExpr();
			}

			ExpectToken(TokenTag::Semicolon);
			return arena_.NewReturnStmt(expr);
		}
		else if (src_.Try(TokenTag::LBrace))
		{
			return ParseCompoundStmt();
		}
		else if (src_.Try(TokenTag::Semicolon))
		{
			// empty statement
			return arena_.NewExprStmt(nullptr);
		}
		else
		{
			if (DetectExpression())
				return ParseExprStmt();
			else
				return ParseDeclStmt();
		}
	}
	StmtBase* ParserImpl::ParseStmt()
	{
		StringVec labels;
		while (src_.LookAhead(1).Tag == TokenTag::Colon)
		{
			ExpectToken(TokenTag::Identifier);
			labels.push_back(src_.LastConsumed().Content);

			ExpectToken(TokenTag::Colon);
		}

		// parse statement without labels
		auto stmt = ParseUnlabelledStmt();
		// assign labels
		copy(labels.begin(), labels.end(), back_inserter(stmt->Labels));

		return stmt;
	}
}