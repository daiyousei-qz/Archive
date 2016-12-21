#pragma once
#include "Basic.h"
#include "Function.h"

namespace SYSTEM_NS
{
	template<typename T>
	class IEnumerable : public Interface
	{
	public:
		virtual void ForEach(Function<void(const T&)>) = 0;
	};
}