#pragma once
#include "AstModel.h"
#include "Type.h"
#include <vector>

namespace lolita
{
	// common base type
	class AstObject
	{
	public:
		AstObject() = default;

		virtual ~AstObject() = default;
	};
}