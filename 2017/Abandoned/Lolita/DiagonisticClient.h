#pragma once
#include "SourceLocation.h"
#include <string>
#include <cstdlib>

namespace lolita
{
	class DiagonisticClient
	{
	public:
		void ReportError(SourceLocation loc, const char* msg)
		{
			auto filename = std::string{ loc.File->Name() };
			printf("error@[line %u; column %u; file %s]: %s\n", loc.Line, loc.Column, filename.c_str(), msg);
		}
	};
}