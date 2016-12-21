#pragma once
#include "ssm_cpu.h"
#include "ssm_module.h"
#include <memory>

class SSMVirtualMachine
{
	static constexpr size_t StackSize = 1 * 1024 * 1024; // 1 MB
	SSMCPU _cpu;
	uint8_t *_stk = new uint8_t[StackSize];

	SSMVirtualMachine(const SSMModule &module)
	{
		_cpu.Initialize(module.ModuleBase, module.InstOffset, _stk);
	}

public:
	~SSMVirtualMachine()
	{
		delete[] _stk;
	}

	bool Run()
	{
		while (_cpu.GetState() == CPUState::Running)
			_cpu.Cycle();

		return _cpu.GetState() == CPUState::Terminated;
	}

	static std::shared_ptr<SSMVirtualMachine> Create(const SSMModule &module)
	{
		SSMVirtualMachine *p = new SSMVirtualMachine(module);
		return std::shared_ptr<SSMVirtualMachine>(p);
	}
};