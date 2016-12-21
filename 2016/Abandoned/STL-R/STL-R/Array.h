#pragma once
#include "Basic.h"
#include "Allocator.h"
#include "IEnumerable.h"

namespace SYSTEM_NS
{
	template<typename T, int Length>
	class FixedArray : public Object, public IEnumerable<T>
	{
		T _body[Length];

	public:
		T &operator[](size_t index)
		{
			Assert(index > 0);
			Assert(index < Length);

			return _body[index];
		}
	};

	template<typename T, typename Allocator = MemoryAllocator<T>>
	class Array : public Object, public IEnumerable<T>
	{
		size_t _len;
		T *_body;

	public:
		explicit Array(size_t sz) : _len(sz), _body(Allocator::Allocate(sz)) { }

		size_t Length() { return _len; }
	};
}