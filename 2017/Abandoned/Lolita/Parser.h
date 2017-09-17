#pragma once
#include "TranslationContext.h"
#include "Token.h"
#include "AstObject.h"
#include "AstArena.h"
#include <memory>
#include <optional>

namespace lolita
{
	class ParseTree
	{
	public:
		using Ptr = unique_ptr<ParseTree>;

		// assuming root belongs to arena
		ParseTree(AstArena arena, AstObject *root)
			: arena_(std::move(arena)), root_(root)
		{
			assert(root_ != nullptr);
		}

		AstObject* Root() const
		{
			return root_;
		}

	private:
		AstArena arena_;
		AstObject *root_;
	};

	ParseTree::Ptr ParseTranslationUnit(const TokenVec& src);
}