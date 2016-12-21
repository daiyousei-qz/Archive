//=============================================================================
// File: <initializer_list>
// Author: Edward Cheng
// Desc: 
//=============================================================================
#pragma once

namespace std 
{
	template<class T>
	class initializer_list
	{
		T *_begin, *_end;
	public:
		// member types
		using value_type = T;
		using reference = const T&;
		using const_reference = const T&;
		using size_type = std::size_t;
		using iterator = const T*;
		using const_iterator = const T*;

		// ctors
		constexpr initializer_list() noexcept : _begin(nullptr), _end(nullptr) { }
		/* this ctor is customized for Microsoft Visual C++ */
		initializer_list(const T *beg, const T *end) noexcept : _begin(beg), _end(end) { }

		// member functions
		size_type size() const noexcept
		{
			return static_cast<size_type>(_end - _begin);
		}

		iterator begin() const noexcept
		{
			return _begin;
		}

		iterator end() const noexcept
		{
			return _end;
		}
	};

}