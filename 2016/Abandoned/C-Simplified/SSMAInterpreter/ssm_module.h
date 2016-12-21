#pragma once
#include "ssm_basic.h"

struct SSMModule
{
	uint8_t *ModuleBase;
	size_t InstOffset;

public:
	SSMModule(uint8_t *base, size_t instOffset) :ModuleBase(base), InstOffset(instOffset) { }

	SSMAOpCode *GetEntry() const
	{
		return reinterpret_cast<SSMAOpCode*>(ModuleBase + InstOffset);
	}

	static SSMModule CreateTest(SSMAOpCode *ip)
	{
		return SSMModule(reinterpret_cast<uint8_t*>(ip), 0);
	}
};