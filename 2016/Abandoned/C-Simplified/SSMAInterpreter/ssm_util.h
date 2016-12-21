#pragma once
#include "ssm_basic.h"

inline bool IsNegative(cpuval_t val)
{
	return (val & (1 << 31)) != 0;
}