#pragma once
#include "SourceFile.h"
#include <cstdint>

namespace lolita
{
	struct SourceLocation
	{
		uint32_t Index;
		uint32_t Line;
		uint32_t Column;

		// nullptr when unspecified
		const SourceFile* File;
	};
}