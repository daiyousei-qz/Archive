//=============================================================================
// File:    <Basic.h>
// Author:  Edward Cheng
// Desc:    Basic definition for STL-R
//=============================================================================
#pragma once
#ifndef STLR_BASIC_H

#define STLR_DEBUG_MODE
#include <cstddef>
#include <type_traits>

namespace eds
{
	//============================================================================
	// basic type definition
	using int8 = signed char;
	using int16 = signed short;
	using int32 = signed long;
	using int64 = signed long long;

	using uint8 = unsigned char;
	using uint16 = unsigned short;
	using uint32 = unsigned long;
	using uint64 = unsigned long long;

	//using size_t = decltype(sizeof(int));
	using nullptr_t = decltype(nullptr);
	using ptrdiff_t = decltype(static_cast<int*>(nullptr) - static_cast<int*>(nullptr));

	// some frequently used type 
	using byte = uint8;

	//============================================================================
	// primary Exception
	class Exception
	{
		const char *_message;

	public:
		Exception(const char *message = "") : _message(message) { }

		const char *Message() const
		{
			return _message;
		}
	};

	//============================================================================
	// assert support

	inline void Assert(bool condition, char *message = "")
	{
		// Assert only enabled in debug mode
#ifdef STLR_DEBUG_MODE
		if (!condition)
			throw Exception(message);
#endif
	}

	//============================================================================
	// specifier removal facility

	// remove reference
	template <typename T>
	struct RemoveReference
	{
		using Type = T;
	};

	template <typename T>
	struct RemoveReference<T&>
	{
		using Type = T;
	};

	template <typename T>
	struct RemoveReference<T&&>
	{
		using Type = T;
	};

	// remove const
	template <typename T>
	struct RemoveConst
	{
		using Type = T;
	};

	template <typename T>
	struct RemoveConst<const T>
	{
		using Type = T;
	};

	// remove volatile
	template <typename T>
	struct RemoveVolatile
	{
		using Type = T;
	};

	template <typename T>
	struct RemoveVolatile<volatile T>
	{
		using Type = T;
	};

	// remove all specifiers
	template <typename T>
	struct RemoveSpecifier
	{
		using Type = T;
	};

	template <typename T>
	struct RemoveSpecifier<T&>
	{
		using Type = RemoveSpecifier<T>;
	};

	template <typename T>
	struct RemoveSpecifier<T&&>
	{
		using Type = RemoveSpecifier<T>;
	};

	template <typename T>
	struct RemoveSpecifier<const T>
	{
		using Type = RemoveSpecifier<T>;
	};

	template <typename T>
	struct RemoveSpecifier<volatile T>
	{
		using Type = RemoveSpecifier<T>;
	};

	//============================================================================
	// reference transformation facility

	template <typename T>
	inline constexpr typename RemoveReference<T>::Type &&Move(T &&obj)
	{
		return static_cast<typename RemoveReference<T>::Type&&>(obj);
	}

	template<typename T>
	inline constexpr T &&Forward(typename RemoveReference<T>::Type &obj)
	{
		return static_cast<T&&>(obj);
	}

	template<typename T>
	inline constexpr T &&Forward(typename RemoveReference<T>::Type &&obj)
	{
		return static_cast<T&&>(obj);
	}

	//============================================================================
	// basic object base

	///<summary>Base type of objects that cannot be constructed.</summary>
	class Unconstructible
	{
	public:
		Unconstructible() = delete;
	};

	///<summary>Base type of objects that cannot be copied.</summary>
	class Uncopyable
	{
	public:
		Uncopyable() = default;
		Uncopyable(const Uncopyable &) = delete;
		Uncopyable &operator =(const Uncopyable &) = delete;
	};

	///<summary>Base class of all.</summary>
	class Object
	{
	public:
		virtual ~Object() { }
	};

	///<summary>Base class for interfaces, noting that interfaces should contain only functions.</summary>
	class Interface : private Uncopyable
	{
	public:
		Interface() = default;
		virtual ~Interface() = default;
	};
}

#endif // !STLR_BASIC_H