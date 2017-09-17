#pragma once
#include "Token.h"
#include "AstModel.h"
#include <optional>

namespace lolita
{
	constexpr inline
	std::optional<UnaryOp> TranslateUnaryOp(TokenTag tag)
	{
		// NOTE increment/decrement is not included for ambiguity
		switch (tag)
		{
		case TokenTag::Ampersand:
			return UnaryOp::Ref;
		case TokenTag::Asterisk:
			return UnaryOp::Deref;
		case TokenTag::Plus:
			return UnaryOp::Plus;
		case TokenTag::Minus:
			return UnaryOp::Minus;
		case TokenTag::Tlide:
			return UnaryOp::BitwiseReverse;
		case TokenTag::Exclamation:
			return UnaryOp::LogicalNot;
		default:
			return std::nullopt;
		}
	}

	constexpr inline
	std::optional<BinaryOp> TranslateBinaryOp(TokenTag tag)
	{
		switch (tag)
		{
			// Non-assignment Ops
			//
		case TokenTag::Plus:
			return BinaryOp::Plus;
		case TokenTag::Minus:
			return BinaryOp::Minus;
		case TokenTag::Asterisk:
			return BinaryOp::Multiply;
		case TokenTag::Slash:
			return BinaryOp::Divide;
		case TokenTag::Modulus:
			return BinaryOp::Modulus;
		case TokenTag::Ampersand:
			return BinaryOp::BitwiseAnd;
		case TokenTag::Pipe:
			return BinaryOp::BitwiseOr;
		case TokenTag::Caret:
			return BinaryOp::BitwiseXor;
		case TokenTag::LShift:
			return BinaryOp::LShift;
		case TokenTag::RShift:
			return BinaryOp::RShift;
		case TokenTag::DoubleAmp:
			return BinaryOp::LogicalAnd;
		case TokenTag::DoublePipe:
			return BinaryOp::LogicalOr;
		case TokenTag::Equal:
			return BinaryOp::Equal;
		case TokenTag::Unequal:
			return BinaryOp::Unequal;
		case TokenTag::Less:
			return BinaryOp::Less;
		case TokenTag::Greater:
			return BinaryOp::Greater;
		case TokenTag::LessEqual:
			return BinaryOp::LessEqual;
		case TokenTag::GreaterEqual:
			return BinaryOp::GreaterEqual;
		case TokenTag::Period:
			return BinaryOp::MemberAccess;
		case TokenTag::Arrow:
			return BinaryOp::PtrMemberAccess;

			// Assignment Ops
			//
		case TokenTag::Assign:
			return BinaryOp::Assign;
		case TokenTag::PlusAssign:
			return BinaryOp::PlusAssign;
		case TokenTag::MinusAssign:
			return BinaryOp::MinusAssign;
		case TokenTag::MulAssign:
			return BinaryOp::MulAssign;
		case TokenTag::DivAssign:
			return BinaryOp::DivAssign;
		case TokenTag::ModAssign:
			return BinaryOp::ModAssign;
		case TokenTag::BitAndAssign:
			return BinaryOp::BitAndAssign;
		case TokenTag::BitOrAssign:
			return BinaryOp::BitOrAssign;
		case TokenTag::BitXorAssign:
			return BinaryOp::BitXorAssign;
		case TokenTag::LShiftAssign:
			return BinaryOp::LShiftAssign;
		case TokenTag::RShiftAssign:
			return BinaryOp::RShiftAssign;

		default:
			return std::nullopt;
		}
	}

	constexpr inline
	bool IsAssignmentOp(BinaryOp op)
	{
		// NOTE this heavily depends on how BinaryOp is defined
		return op >= BinaryOp::Assign;
	}
}