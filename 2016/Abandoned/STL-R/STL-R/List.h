#pragma once
#include "Basic.h"
#include "Allocator.h"
#include "Array.h"
#include "IEnumerable.h"

namespace SYSTEM_NS
{
	///<summary>Linear List</summary>
	template<typename T, typename Allocator = MemoryAllocator<T>>
	class List : public Uncopyable, public IEnumerable<T>
	{
		static constexpr size_t DefaultInitialCapacity = 8;
		Array<T, Allocator> _array;
		size_t _count;
		uint32 _version = 0;

	public:
		List(size_t capacity) : _target(Allocator::Allocate(capacity)),
								_count(0),
								_capacity(_capacity)
		{ }

		List() : List(DefaultInitialCapacity)
		{

		}

		size_t Count() { return _count; }
		size_t Capacity() { return _capacity; }

		void Reserve(size_t sz);
		void Push(const T& ins);
		T &&Pop();

		size_t IndexOf(const T& ins);
		bool Contain(const T& ins);
	};
}