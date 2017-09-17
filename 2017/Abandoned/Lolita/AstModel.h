#pragma once
#include "EnumUtils.h"

namespace lolita
{
	// Parser in charge of translating C representation
	enum class BuiltinType
	{
		Void,

		Boolean,
		Char,

		Int8,
		Int16,
		Int32,
		Int64,

		UInt8,
		UInt16,
		UInt32,
		UInt64,

		Float32,
		Float64,
	};

	struct TypeQualifier
	{
		bool Const = false;
		bool Restrict = false;
		bool Volatile = false;
	};

	struct FunctionSpecifier
	{
		bool Inline = false;
		bool Noreturn = false;
	};

	// NOTE though _Thread_local is a storage specifier
	// it is processed saperately
	// NOTE though typedef is not a storage specifier
	// it is treated as one
	enum class StorageSpecifier
	{
		None,
		Auto,
		Register,
		Static,
		Extern,
		Typedef,
		// ThreadLocal
	};

	enum class UnaryOp
	{
		FrontInc,
		FrontDec,
		BackInc,
		BackDec,

		Ref,
		Deref,
		Plus,
		Minus,
		BitwiseReverse,
		LogicalNot,
	};

	enum class BinaryOp
	{
		// Arithmetic
		Plus,
		Minus,
		Multiply,
		Divide,
		Modulus,

		// Bitwise Op
		BitwiseAnd,
		BitwiseOr,
		BitwiseXor,
		LShift,
		RShift,

		// Logical Op
		LogicalOr,
		LogicalAnd,

		// Comparison
		Equal,
		Unequal,
		Less,
		Greater,
		LessEqual,
		GreaterEqual,

		// Access
		MemberAccess,
		PtrMemberAccess,

		// Assignment
		Assign,
		PlusAssign,
		MinusAssign,
		MulAssign,
		DivAssign,
		ModAssign,
		BitAndAssign,
		BitOrAssign,
		BitXorAssign,
		LShiftAssign,
		RShiftAssign,
	};

	enum class JumpStrategy
	{
		Break,
		Continue,
		Goto,
	};
}