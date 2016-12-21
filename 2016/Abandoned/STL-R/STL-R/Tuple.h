#pragma once
#ifndef STLR_TUPLE_H
#define STLR_TUPLE_H

#include "Basic.h"

namespace eds
{
	template <typename ... TArgs>
	struct Tuple { };

	template<typename T1>
	struct Tuple<T1>
	{
		T1 First;
	};

	template<typename T1,
			 typename T2>
	struct Tuple<T1, T2>
	{
		T1 First;
		T2 Second;
	};

	template<typename T1,
			 typename T2,
			 typename T3>
	struct Tuple<T1, T2, T3>
	{
		T1 First;
		T2 Second;
		T3 Third;
	};
}

#endif //!STLR_TUPLE_H