#pragma once
#include <type_traits>

namespace lolita
{
	namespace detail
	{
		// custom type trait to identify an enumeration flags
		template <typename EnumType>
		struct IsEnumFlag : std::false_type {};
	}

	// NOTE this only works inside namespace lolita
	// NOTE as this file is included, <type_traits> is included as well
#define ALLOW_ENUM_FLAGS(ENUMTYPE) \
namespace detail { template <> struct IsEnumFlag<ENUMTYPE> : std::true_type {}; }

	// A wrapper for scoped enumeration
	// to provide bitmask flags facilities
	// NOTE this wrapper assumes 0 as empty
	template <typename EnumType>
	class EnumFlags
	{
	private:
		static_assert(detail::IsEnumFlag<EnumType>::value,
			"You have to use macro ALLOW_ENUM_FLAGS to enable instantiation.");
		static_assert(std::is_enum_v<EnumType>,
			"Template parameter of EnumFlags must be an enumeration.");

		// NOTE BaseType is always an arithmetic type
		using UnderlyingType = std::underlying_type_t<EnumType>;

		UnderlyingType value_;

	public:
		// Make this type POD
		// NOTE usually the default constructed object has an invalid value
		constexpr EnumFlags() = default;
		constexpr EnumFlags(EnumType value)
			: value_(static_cast<UnderlyingType>(value)) { }

		constexpr bool Empty() const noexcept
		{
			return value_ == 0;
		}

		constexpr void Clear() const noexcept
		{
			value_ = 0;
		}

		constexpr bool Test(EnumFlags flags) const noexcept
		{
			return !(*this & flags).Empty();
		}

		constexpr EnumFlags& operator|=(EnumFlags other) noexcept
		{
			value_ |= other.value_;
			return *this;
		}
		constexpr EnumFlags& operator&=(EnumFlags other) noexcept
		{
			value_ &= other.value_;
			return *this;
		}
	};

	template <typename E>
	constexpr auto operator|(EnumFlags<E> lhs, EnumFlags<E> rhs) noexcept
	{
		auto result = lhs;
		result |= rhs;

		return result;
	}
	template <typename E>
	constexpr auto operator&(EnumFlags<E> lhs, EnumFlags<E> rhs) noexcept
	{
		auto result = lhs;
		result &= rhs;

		return result;
	}

	template <typename E>
	constexpr auto operator|(E lhs, E rhs) noexcept
		-> std::enable_if_t<detail::IsEnumFlag<E>::value, EnumFlags<E>>
	{
		auto result = EnumFlags<E>{ lhs };
		result |= rhs;

		return result;
	}

	template <typename E>
	constexpr auto operator&(E lhs, E rhs) noexcept
		-> std::enable_if_t<detail::IsEnumFlag<E>::value, EnumFlags<E>>
	{
		auto result = EnumFlags<E>{ lhs };
		result &= rhs;

		return result;
	}
}