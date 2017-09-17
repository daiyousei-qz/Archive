#pragma once
#include <cstdint>
#include <string>
#include <variant>

namespace lolita
{
	class CInteger;
	class CFloat;
	class CString;

	// Integer
	//

	enum class IntPrecision
	{
		Int8,
		Int16,
		Int32,
		Int64,

		UInt8,
		UInt16,
		UInt32,
		UInt64,
	};

	class CInteger
	{
	public:
		// value maybe truncated or extended according to the particular precision
		CInteger(uint64_t value, IntPrecision p);

		CInteger Cast(IntPrecision new_type) const
		{
			return CInteger(value_, new_type);
		}

		// return value evaluated as an unsigned integer
		// even if precision indicates it's a signed integer
		uint64_t Value() const
		{
			return value_;
		}

		IntPrecision Precision() const
		{
			return precision_;
		}

	private:
		uint64_t value_;
		IntPrecision precision_;
	};

	// Float
	//

	enum class FloatPrecision
	{
		Single,
		Double,

		// ExDouble,
	};

	class CFloat
	{
	public:
		// value maybe truncated or extended according to the particular precision
		CFloat(double value, FloatPrecision p);

		CFloat Cast(FloatPrecision new_type) const
		{
			return CFloat(value_, new_type);
		}

		double Value() const
		{
			return value_;
		}

		FloatPrecision Precision() const
		{
			return precision_;
		}

	private:
		double value_;
		FloatPrecision precision_;
	};

	// String
	//

	enum class EncodingType
	{
		Char,
		Char16,
		Char32,
		WChar,
	};

	class CString
	{
	public:
		CString(const std::string& value, EncodingType type)
			: text_(value), encoding_(type) { }

		const std::string& Value() const
		{
			return text_;
		}

		EncodingType Encoding() const
		{
			return encoding_;
		}

	private:
		// Text in UTF-8
		std::string text_;
		// Target encoding type
		EncodingType encoding_;
	};

	// A Unified Constant Type
	//

	using CConstant = std::variant<std::monostate, CInteger, CFloat, CString>;
}