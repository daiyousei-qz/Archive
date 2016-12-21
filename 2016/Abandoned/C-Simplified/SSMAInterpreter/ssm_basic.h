#pragma once

// ASSERT
#define SSMA_DEBUG
inline void _Assert(bool condition)
{
#ifdef SSMA_DEBUG
	if (!condition)
	{
		throw 0;
	}
#endif
}

// basic type definition
using int8_t = signed char;
using uint8_t = unsigned char;
using int16_t = signed short;
using uint16_t = unsigned short;
using int32_t = signed int;
using uint32_t = unsigned int;
static_assert(sizeof(int8_t) == 1, " ");
static_assert(sizeof(int16_t) == 2, " ");
static_assert(sizeof(int32_t) == 4, " ");
static_assert(sizeof(uint8_t) == 1, " ");
static_assert(sizeof(uint16_t) == 2, " ");
static_assert(sizeof(uint32_t) == 4, " ");

using cpuval_t = uint32_t;

// cpu metadata
enum class CPUState
{
	Terminated,
	Running,
	Thrown
};

enum class Register : uint8_t
{
	RIP = 0, // Pointer to next to be run instruction
	RSB = 1, // Pointer to the base of the current stack frame
	RST = 2, // Pointer to the top of the current stack frame
	RTD = 3, // Temporary data storage, usually for argument and return value passing
	RMB = 4, // Pointer to base of the current module
};

enum SSMAOpCode : uint8_t
{
	term = 0,
	nop = 1,

	add = 2,
	sub = 3,
	mul = 4,
	div_ = 5,
	mod = 6,
	b_and = 7,
	b_or = 8,
	b_xor = 9,
	b_rev = 10,

	push_c0 = 20,
	push_c1 = 21,
	push_1 = 22, // [1+1] push a byte into ES
	push_4 = 23, // [1+4] push an 32-bit int into ES
	push_r = 24, // [1+1] push the value of a specific register into ES
	push_ra = 25,
	pop = 26,
	swap = 27,
	dup = 28,
	rec = 29,
	store_1 = 30,
	store_2 = 31,
	store_4 = 32,
	load_1 = 33,
	load_2 = 34,
	load_4 = 35,
	alloc = 36,

	jmp = 40, // [1+2] append an offset of 16-bit int to $RCP
	jmpz = 41, // [1+2] jmp if 0 on the top of ES, and pop
	jmpn = 42, // [1+2] jmp if negative on the top of ES, and pop
	call = 43,
	ret = 44,
};

constexpr size_t RegisterCount = 5;
constexpr size_t ESSize = 6;
constexpr size_t CallerInfoSize = 40;


// note this struct cannot be constructed
// they're only used when refer to opcodes
// so any alignment is cancelled
#pragma pack(push)
#pragma pack(1)
struct OP_V
{
	SSMAOpCode Code;

	OP_V() = delete;
	OP_V(OP_V &) = delete;
	OP_V(OP_V &&) = delete;

	template <typename T>
	const T *As() const
	{
		return reinterpret_cast<const T*>(this);
	}
};

template <typename T>
struct OP_1 : OP_V
{
	T Operand1;

	OP_1() = delete;
	OP_1(OP_1 &) = delete;
	OP_1(OP_1 &&) = delete;
};

using OP_S1 = OP_1<int8_t>;
using OP_S2 = OP_1<int16_t>;
using OP_S4 = OP_1<int32_t>;
using OP_U1 = OP_1<uint8_t>;
using OP_U2 = OP_1<uint16_t>;
using OP_U4 = OP_1<uint32_t>;

template <typename T, typename U>
struct OP_2 : OP_V
{
	T Operand1;
	U Operand2;

	OP_2() = delete;
	OP_2(OP_2 &) = delete;
	OP_2(OP_2 &&) = delete;
};

using OP_U1U2 = OP_2<uint8_t, uint16_t>;

struct FrameCallerInfo
{
	cpuval_t RMB;
	cpuval_t RIP;
	cpuval_t RSB;
	cpuval_t ESIndex;
	cpuval_t ES[ESSize];
};
#pragma pack(pop)