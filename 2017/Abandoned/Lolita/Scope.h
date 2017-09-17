#pragma once
#include "AstObject.h"
#include "Type.h"
#include <memory>
#include <map>

namespace lolita
{
	enum class ScopeCategory
	{
		File,
		Function,
		Block,
		FunctionPrototype,
	};

	// what an ordianary identifier refers to (variables, functions or typedef names)
	struct NamedEntity : public AstObject
	{
		std::string Name;

		QualType Type;

		StorageSpecifier StorageClass;
	};

	// an abstract of a scope in C language
	class Scope : public AstObject
	{
	public:
		Scope(Scope* parent, ScopeCategory cat)
			: parent_(parent), category_(cat) { }

		ScopeCategory Category() const { return category_; }
		Scope* Parent() const { return parent_; }

		// CType* LookupType(const std::string& name);
		// AggregateType* DeclareStruct(const std::string& name, AggregationLayout layout);

		void DeclareEntity(NamedEntity item);
		void DeclareEntity(const std::string& name, QualType type, StorageSpecifier storage);
		const NamedEntity* LookupEntity(const std::string& name);

	private:
		const ScopeCategory category_;
		Scope *const parent_;

		// declaration of types in the current scope
		// in Tag name space
		std::map<std::string, CType*> types_;

		// in Ordianary name space
		std::map<std::string, NamedEntity> decls_;
	};
}