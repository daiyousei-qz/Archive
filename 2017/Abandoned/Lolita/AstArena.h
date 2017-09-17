#pragma once
#include "AstObject.h"
#include "Expr.h"
#include "Stmt.h"
#include "Decl.h"
#include "Scope.h"
#include "Literal.h"
#include <memory>
#include <deque>

namespace lolita
{
	// Arena(memory management)
	//

	// Guard object that manages the lifetime of AstObject
	// when this object goes out of the scope, all its properties would be discarded
	class AstArena
	{
	public:
		using Ptr = std::unique_ptr<AstArena>;

		// default ctor
		AstArena() = default;

		// Decl Factory
		//
		DeclBase* NewGroupDecl(const std::vector<DeclBase*>& group);
		DeclBase* NewVariableDecl(QualType type, const std::string& name, ExprBase* init);
		DeclBase* NewFunctionDecl(CType* type, const std::string& name, StmtBase* body);

		// Scope Factory
		//
		Scope* NewScope(Scope* parent, ScopeCategory cat);

		template <typename T, typename ... TArgs>
		T* MakeAstObject(TArgs&& ...args)
		{
			auto ptr = std::make_unique<T>(std::forward<TArgs>(args)...);
			auto result = ptr.get();
			
			allocated_.emplace_back(std::move(ptr));
			return result;
		}

	private:
		std::deque<std::unique_ptr<AstObject>> allocated_;
	};
}