#include "ParserImpl.h"

namespace lolita
{
	DeclBase* ParserImpl::ParseStaticAssert()
	{
		throw 0;
	}

	DeclBase* ParserImpl::ParseDeclaration(FunctionSpecifier* func, StorageSpecifier* storage)
	{
		auto base_type = ParseDeclSpec(func, storage, nullptr, nullptr);

		bool first_run = true;
		std::vector<DeclBase*> decls;
		do
		{
			StringVec param_annot;
			auto decl_type = ParseDeclarator(base_type, &param_annot);
			assert(decl_type.second.has_value());

			// function definition
			if (dynamic_cast<FunctionType*>(decl_type.first.Type)
				&& src_.Test(TokenTag::LBrace))
			{
				if (!first_run)
				{
					ReportUnexpectedToken(src_.Peek());
					throw ParsingError{};
				}

				// function definition
				src_.Consume(); // consume {
				auto body = ParseCompoundStmt();
				return arena_.NewFunctionDecl(decl_type.first.Type, *decl_type.second, body);
			}

			auto init = static_cast<ExprBase*>(nullptr);
			if (src_.Try(TokenTag::Assign))
			{
				init = ParseAssignmentExpr();
			}

			decls.push_back(arena_.NewVariableDecl(decl_type.first, *decl_type.second, init));
			first_run = false;
		} while (src_.Try(TokenTag::Comma));

		ExpectToken(TokenTag::Semicolon);

		// FIXME: add specifier
		if (decls.size() == 1)
		{
			return decls.front();
		}
		else
		{
			return arena_.NewGroupDecl(decls);
		}
	}

	std::vector<DeclBase*> ParserImpl::ParseTranslationUnit()
	{
		std::vector<DeclBase*> result;

		try
		{
			while (!src_.Exhausted())
			{
				if (src_.Try(TokenTag::Semicolon))
					continue;

				if (src_.Try(TokenTag::StaticAssert))
				{
					result.push_back(ParseStaticAssert());
				}
				else
				{
					result.push_back(ParseDeclaration());
				}
			}
		}
		catch (const ParsingError& error)
		{
			AbortTranslation();
		}

		return result;
	}
}