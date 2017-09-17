#include "AstArena.h"

using namespace std;

namespace lolita
{



	// Decl Factory
	//
	DeclBase* AstArena::NewGroupDecl(const std::vector<DeclBase*>& group)
	{
		auto result = MakeAstObject<GroupDecl>();
		result->Group = group;

		return result;
	}
	DeclBase* AstArena::NewVariableDecl(QualType type, const std::string& name, ExprBase* init)
	{
		auto result = MakeAstObject<VariableDecl>();
		result->Type = type;
		result->Name = name;
		result->Init = init;

		return result;
	}
	DeclBase* AstArena::NewFunctionDecl(CType* type, const std::string& name, StmtBase* body)
	{
		auto result = MakeAstObject<FunctionDecl>();
		result->Type = type;
		result->Name = name;
		result->Body = body;

		return result;
	}

	// Scope Factory
	//
	Scope* AstArena::NewScope(Scope* parent, ScopeCategory cat)
	{
		return MakeAstObject<Scope>(parent, cat);
	}
}