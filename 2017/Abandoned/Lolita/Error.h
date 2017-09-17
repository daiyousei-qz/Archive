#pragma once
#include "SourceLocation.h"
#include <stdexcept>

namespace lolita
{
	class CompilationError : std::runtime_error
	{
	public:
		CompilationError(const char* msg = "") : std::runtime_error(msg) { }
	};

	// throw this error when some devestating error is found
	class CompilerFatalError : CompilationError
	{
	public:
		CompilerFatalError() : CompilationError() { }
	};

	class PreprocessingError : CompilationError
	{
	public:
		PreprocessingError() : CompilationError() { }
	};

	class ParsingError : CompilationError
	{
	public:
		ParsingError() : CompilationError() { }
	};

	class LiteralError : CompilationError
	{
	public:
		LiteralError(const char* msg = "")
			: CompilationError(msg) { }
	};

}