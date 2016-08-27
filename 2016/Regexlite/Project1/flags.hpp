#pragma once
#include <type_traits>

namespace eds {

namespace detail {

template <typename T>
struct IsFlagEnum : std::false_type {};

}

#define DEFINE_FLAG_ENUM(ENUM_TYPENAME) \
static_assert(std::is_enum<TEnum>::value, "!!!"); \
template <> struct detail::IsFlagEnum<ENUM_TYPENAME> : std::true_type { };

template <typename TEnum,
          typename = std::enable_if_t<detail::IsFlagEnum<TEnum>::value>>
TEnum operator|(TEnum lhs, TEnum rhs)
{
    return static_cast<TEnum>(static_cast<int>(lhs)| static_cast<int>(rhs));
}
template <typename TEnum,
          typename = std::enable_if_t<detail::IsFlagEnum<TEnum>::value>>
TEnum operator|=(TEnum &lhs, TEnum rhs)
{
    lhs = operator|(lhs, rhs);
}
template <typename TEnum,
    typename = std::enable_if_t<detail::IsFlagEnum<TEnum>::value>>
bool operator&(TEnum lhs, TEnum rhs)
{
    return lhs == operator|(lhs, rhs);
}

}