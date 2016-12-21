#pragma once
#include "ssm_basic.h"
#include "ssm_util.h"

class SSMCPU
{
	cpuval_t _reg[RegisterCount];
	cpuval_t _es[ESSize];
	size_t _esIndex = 0;
	CPUState _state = CPUState::Terminated;

	// Unwrap a register as a cpuval_t
	cpuval_t &_Unwrap(Register reg);
	// Fetch an instruction and advance $RIP
	//   this function returns a pointer to an opcode without operands
	//   users may need to reinterpret the pointer
	OP_V *_Fetch();
	// Append offset on $RIP 
	void _Jump(int16_t offset);
	// Push a value into ES(Evaluation Stack)
	void _ESPush(cpuval_t val);
	// Pop a value from ES(Evaluation Stack) 
	cpuval_t _ESPop();
	// Peek the top of ES(Evaluation Stack)
	cpuval_t _ESPeek();

public:
	// construct and initialize
	SSMCPU(uint8_t *ip, size_t instOffset, uint8_t *sp)
	{
		Initialize(ip, instOffset, sp);
	}

	// construct an uninitialized cpu instance
	SSMCPU() = default;
	SSMCPU(SSMCPU &) = delete;
	SSMCPU(SSMCPU &&) = delete;

	CPUState GetState() { return _state; }
	void Initialize(uint8_t *ip, size_t instOffset, uint8_t *sp);
	// Execute an instruction and move forward
	void Cycle();
};

const class OpcodeSizeTable
{
	size_t _table[256]{}; // zero initialized
public:
	OpcodeSizeTable()
	{
		_table[SSMAOpCode::term] = sizeof(OP_V);
		_table[SSMAOpCode::nop] = sizeof(OP_V);

		_table[SSMAOpCode::add] = sizeof(OP_V);
		_table[SSMAOpCode::sub] = sizeof(OP_V);
		_table[SSMAOpCode::mul] = sizeof(OP_V);
		_table[SSMAOpCode::div_] = sizeof(OP_V);
		_table[SSMAOpCode::mod] = sizeof(OP_V);
		_table[SSMAOpCode::b_and] = sizeof(OP_V);
		_table[SSMAOpCode::b_or] = sizeof(OP_V);
		_table[SSMAOpCode::b_xor] = sizeof(OP_V);
		_table[SSMAOpCode::b_rev] = sizeof(OP_V);

		_table[SSMAOpCode::push_c0] = sizeof(OP_V);
		_table[SSMAOpCode::push_c1] = sizeof(OP_V);
		_table[SSMAOpCode::push_1] = sizeof(OP_U1);
		_table[SSMAOpCode::push_4] = sizeof(OP_U4);
		_table[SSMAOpCode::push_r] = sizeof(OP_U1);
		_table[SSMAOpCode::push_ra] = sizeof(OP_U1U2);
		_table[SSMAOpCode::pop] = sizeof(OP_V);
		_table[SSMAOpCode::swap] = sizeof(OP_V);
		_table[SSMAOpCode::dup] = sizeof(OP_V);
		_table[SSMAOpCode::rec] = sizeof(OP_V);
		_table[SSMAOpCode::store_1] = sizeof(OP_V);
		_table[SSMAOpCode::store_2] = sizeof(OP_V);
		_table[SSMAOpCode::store_4] = sizeof(OP_V);
		_table[SSMAOpCode::load_1] = sizeof(OP_V);
		_table[SSMAOpCode::load_2] = sizeof(OP_V);
		_table[SSMAOpCode::load_4] = sizeof(OP_V);
		_table[SSMAOpCode::alloc] = sizeof(OP_U2);

		_table[SSMAOpCode::jmp] = sizeof(OP_S2);
		_table[SSMAOpCode::jmpz] = sizeof(OP_S2);
		_table[SSMAOpCode::jmpn] = sizeof(OP_S2);
		_table[SSMAOpCode::call] = sizeof(OP_U4);
		_table[SSMAOpCode::ret] = sizeof(OP_V);

	}

	const size_t operator[](SSMAOpCode op) const
	{
		return _table[op];
	}
} _OpSize;

//==========================================================================
// impl. for SSMACPU

inline cpuval_t &SSMCPU::_Unwrap(Register reg)
{
	// assert reg is valid
	_Assert(static_cast<uint8_t>(reg) >= 0);
	_Assert(static_cast<uint8_t>(reg) < RegisterCount);

	return _reg[static_cast<uint8_t>(reg)];
}

inline OP_V *SSMCPU::_Fetch()
{
	// retrive $RIP
	auto &&rip = reinterpret_cast<OP_V*>(_Unwrap(Register::RIP));
	// advance $RIP
	_Unwrap(Register::RIP) += _OpSize[rip->Code];
	// return reinterpreted $RIP 
	return rip;
}

inline void SSMCPU::_Jump(int16_t offset)
{
	_Unwrap(Register::RIP) += offset;
}

inline void SSMCPU::_ESPush(cpuval_t val)
{
	_Assert(_esIndex < ESSize);

	_es[_esIndex++] = val;
}

inline cpuval_t SSMCPU::_ESPop()
{
	_Assert(_esIndex > 0);

	return _es[--_esIndex];
}

inline cpuval_t SSMCPU::_ESPeek()
{
	_Assert(_esIndex > 0);

	return _es[_esIndex - 1];
}

inline void SSMCPU::Initialize(uint8_t *ip, size_t instOffset, uint8_t *sp)
{
	// initialize registers
	_Unwrap(Register::RIP) = reinterpret_cast<cpuval_t>(ip) + instOffset;
	_Unwrap(Register::RSB) = reinterpret_cast<cpuval_t>(sp);
	_Unwrap(Register::RST) = reinterpret_cast<cpuval_t>(sp);
	_Unwrap(Register::RTD) = 0;
	_Unwrap(Register::RMB) = reinterpret_cast<cpuval_t>(ip);

	// initialize Evaluation Stack
	// ...

	_state = CPUState::Running;
}

// opcode is interpreted via switch clause
inline void SSMCPU::Cycle()
{
	_Assert(_state == CPUState::Running);

	// fetch instruction and move forward
	const auto &&op = _Fetch();

	// execute instruction
	switch (op->Code)
	{
	// termination
	case SSMAOpCode::term:
		_state = CPUState::Terminated;
		break;
	// nop
	case SSMAOpCode::nop:
		break;

	// arithmetic operations
	case SSMAOpCode::add:
		_ESPush(_ESPop() + _ESPop());
		break;
	case SSMAOpCode::sub:
	{
		auto &&n1 = _ESPop();
		auto &&n2 = _ESPop();
		_ESPush(n2 - n1);
		break;
	}
	case SSMAOpCode::mul:
		_ESPush(_ESPop() * _ESPop());
		break;
	case SSMAOpCode::div_:
	{
		auto &&n1 = _ESPop();
		auto &&n2 = _ESPop();
		_ESPush(n2 / n1);
		break;
	}
	case SSMAOpCode::mod:
	{
		auto &&n1 = _ESPop();
		auto &&n2 = _ESPop();
		_ESPush(n2 % n1);
		break;
	}
	case SSMAOpCode::b_and:
		_ESPush(_ESPop() & _ESPop());
		break;
	case SSMAOpCode::b_or:
		_ESPush(_ESPop() | _ESPop());
		break;
	case SSMAOpCode::b_xor:
		_ESPush(_ESPop() ^ _ESPop());
		break;
	case SSMAOpCode::b_rev:
		_ESPush(~_ESPop());
		break;

	// ES operations
	case SSMAOpCode::push_c0:
		_ESPush(0);
		break;
	case SSMAOpCode::push_c1:
		_ESPush(1);
		break;
	case SSMAOpCode::push_1:
		_ESPush(op->As<OP_U1>()->Operand1);
		break;
	case SSMAOpCode::push_r:
		_ESPush(_Unwrap(static_cast<Register>(op->As<OP_U1>()->Operand1)));
		break;
	case SSMAOpCode::push_ra:
	{
		auto &&_op = op->As<OP_U1U2>();
		_ESPush(_Unwrap(static_cast<Register>(_op->Operand1)) + _op->Operand2);
		break;
	}
	case SSMAOpCode::push_4:
		_ESPush(op->As<OP_U4>()->Operand1);
		break;
	case SSMAOpCode::pop:
		_ESPop();
		break;
	case SSMAOpCode::swap:
	{
		cpuval_t &&tmp1 = _ESPop();
		cpuval_t &&tmp2 = _ESPop();
		_ESPush(tmp1);
		_ESPush(tmp2);
		break;
	}
	case SSMAOpCode::dup:
		_ESPush(_ESPeek());
		break;
	case SSMAOpCode::rec:
		_Unwrap(Register::RTD) = _ESPop();
		break;
	case SSMAOpCode::store_1:
	{
		uint8_t *&&p = reinterpret_cast<uint8_t*>(_ESPop());
		*p = _ESPop();
		break;
	}
	case SSMAOpCode::store_2:
	{
		uint16_t *&&p = reinterpret_cast<uint16_t*>(_ESPop());
		*p = _ESPop();
		break;
	}
	case SSMAOpCode::store_4:
	{
		uint32_t *&&p = reinterpret_cast<uint32_t*>(_ESPop());
		*p = _ESPop();
		break;
	}
	case SSMAOpCode::load_1:
	{
		uint8_t *&&p = reinterpret_cast<uint8_t*>(_ESPop());
		_ESPush(*p);
		break;
	}
	case SSMAOpCode::load_2:
	{
		uint16_t *&&p = reinterpret_cast<uint16_t*>(_ESPop());
		_ESPush(*p);
	}
		break;	
	case SSMAOpCode::load_4:
	{
		uint32_t *&&p = reinterpret_cast<uint32_t*>(_ESPop());
		_ESPush(*p);
		break;
	}
	case SSMAOpCode::alloc:
		_Unwrap(Register::RST) += reinterpret_cast<OP_U2*>(op)->Operand1;
		break;

		// flow control
	case SSMAOpCode::jmp:
		_Jump(op->As<OP_S2>()->Operand1);
		break;
	case SSMAOpCode::jmpz:
		if (_ESPop() == 0)
			_Jump(op->As<OP_S2>()->Operand1);
		break;
	case SSMAOpCode::jmpn:
		if (IsNegative(_ESPop()))
			_Jump(op->As<OP_S2>()->Operand1);
		break;
	case SSMAOpCode::call:
	{
		// caller info
		// $RSB
		// $RIP
		// $RMB
		// ES Index
		// ES
		FrameCallerInfo *info = reinterpret_cast<FrameCallerInfo*>(_Unwrap(Register::RST));
		info->RSB = _Unwrap(Register::RSB);
		info->RIP = _Unwrap(Register::RIP);
		info->RMB = _Unwrap(Register::RMB);
		info->ESIndex = _esIndex;
		for (int i = 0; i < ESSize;++i)
			info->ES[i] = _es[i];

		auto &&stk_pval = reinterpret_cast<cpuval_t>(info + 1);
		_Unwrap(Register::RSB) = stk_pval;
		_Unwrap(Register::RST) = stk_pval;
		_Unwrap(Register::RIP) = _Unwrap(Register::RMB) + op->As<OP_U4>()->Operand1;
		_esIndex = 0;

		break;
	}
	case SSMAOpCode::ret:
	{
		// caller info
		// $RSB
		// $RIP
		// $RMB
		// ES Index
		// ES
		FrameCallerInfo *info = reinterpret_cast<FrameCallerInfo*>(_Unwrap(Register::RSB)) - 1;
		_Unwrap(Register::RIP) = info->RIP;
		_Unwrap(Register::RSB) = info->RSB;
		_Unwrap(Register::RMB) = info->RMB;
		_Unwrap(Register::RST) = reinterpret_cast<cpuval_t>(info);
		_esIndex = info->ESIndex;
		for (int i = 0; i < ESSize; ++i)
			_es[i] = info->ES[i];

		break;
	}

	default:
		throw 0;
	}
}