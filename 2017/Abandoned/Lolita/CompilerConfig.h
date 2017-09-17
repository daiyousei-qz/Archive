#pragma once

namespace lolita
{
	// Constants 
	//

	static constexpr size_t kLexerTabSize = 4;

	static constexpr size_t kTypeCharSize = 1;
	static constexpr size_t kTypeShortSize = 2 * kTypeCharSize;
	static constexpr size_t kTypeIntSize = 4 * kTypeCharSize;
	static constexpr size_t kTypeLongSize = 4 * kTypeCharSize;
	static constexpr size_t kTypeLongLongSize = 8 * kTypeCharSize;

	static constexpr size_t kPointerSize = kTypeIntSize;
}