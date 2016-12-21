#pragma once
#include "ssm_basic.h"

class MemoryBus
{
	template <typename T>
	T &Access(cpuval_t p);

	OP_V *FetchOp(cpuval_t p);
};