#pragma once
#include "Basic.h"

namespace SYSTEM_NS
{
	template<typename T>
	class MemoryAllocator : Unconstructible
	{
	public:
		static T *Allocate(size_t cnt) { throw Exception(); }
		static void Deallocate(T* p) { throw Exception(); }
	};
}