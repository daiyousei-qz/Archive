#include "Scope.h"

namespace lolita
{
	// Implementation of Scope
	//

	void Scope::DeclareEntity(NamedEntity item)
	{
		assert(!LookupEntity(item.Name));
		decls_[item.Name] = item;
	}
	void Scope::DeclareEntity(const std::string& name, QualType type, StorageSpecifier storage)
	{
		assert(!LookupEntity(name));
		decls_[name] = NamedEntity { name, type, storage };
	}

	const NamedEntity* Scope::LookupEntity(const std::string& name)
	{
		auto iter = decls_.find(name);
		if (iter != decls_.end())
		{
			return &iter->second;
		}
		else
		{
			return parent_
				? parent_->LookupEntity(name)
				: nullptr;
		}
	}
}