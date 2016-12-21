#include "Basic.h"

namespace eds
{
	// basic arithmetic type size assert
	static constexpr size_t BYTE_WIDTH = sizeof(char);

	static_assert(sizeof(int8) == BYTE_WIDTH, "");
	static_assert(sizeof(int16) == 2 * BYTE_WIDTH, "");
	static_assert(sizeof(int32) == 4 * BYTE_WIDTH, "");
	static_assert(sizeof(int64) == 8 * BYTE_WIDTH, "");

	static_assert(sizeof(uint8) == BYTE_WIDTH, "");
	static_assert(sizeof(uint16) == 2 * BYTE_WIDTH, "");
	static_assert(sizeof(uint32) == 4 * BYTE_WIDTH, "");
	static_assert(sizeof(uint64) == 8 * BYTE_WIDTH, "");
}