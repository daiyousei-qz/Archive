#include "Literal.h"

namespace lolita
{
	// store integer in little endian
	// assuming compiled at x86 platform
	union IntPadding
	{
		int8_t value_i8_;
		int16_t value_i16_;
		int32_t value_i32_;
		int64_t value_i64_;

		uint8_t value_u8_;
		uint16_t value_u16_;
		uint32_t value_u32_;
		uint64_t value_u64_;

		unsigned char dummy_[8];
	};

	CInteger::CInteger(uint64_t value, IntPrecision p)
		: precision_(p)
	{
		IntPadding padding;

		// initialize padding to 0
		padding.value_u64_ = 0;

		// cast type
		switch (p)
		{
		case IntPrecision::Int8:
			padding.value_i8_ = value;
			break;
		case IntPrecision::Int16:
			padding.value_i16_ = value;
			break;
		case IntPrecision::Int32:
			padding.value_i32_ = value;
			break;
		case IntPrecision::Int64:
			padding.value_i64_ = value;
			break;
		case IntPrecision::UInt8:
			padding.value_u8_ = value;
			break;
		case IntPrecision::UInt16:
			padding.value_u16_ = value;
			break;
		case IntPrecision::UInt32:
			padding.value_u32_ = value;
			break;
		case IntPrecision::UInt64:
			padding.value_u64_ = value;
			break;
		}

		value_ = padding.value_u64_;
	}

	CFloat::CFloat(double value, FloatPrecision p)
		: precision_(p)
	{
		value_ = value;
	}
}