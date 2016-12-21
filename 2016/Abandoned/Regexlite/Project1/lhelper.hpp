/*=================================================================================
*  Copyright (c) 2016 Edward Cheng
*
*  edslib is an open-source library in C++ and licensed under the MIT License.
*  Refer to: https://opensource.org/licenses/MIT
*================================================================================*/

// This file is modefied from Microsoft's implementation of GSL:
// refer to https ://github.com/Microsoft/GSL`

#pragma once
#ifndef LH_CONTRACT_H
#define LH_CONTRACT_H

#include <exception>
#include <stdexcept>
#include <type_traits>

#define LH_STRINGIFY_DETAIL(x) #x
#define LH_STRINGIFY(x) LH_STRINGIFY_DETAIL(x)

// 
// There are three configuration options for this lh implementation's behavior 
// when pre/post conditions on the lh types are violated: 
// 
// 1. LH_TERMINATE_ON_CONTRACT_VIOLATION: std::terminate will be called
// 2. LH_THROW_ON_CONTRACT_VIOLATION: a lh::fail_fast exception will be thrown (default)
// 3. LH_UNENFORCED_ON_CONTRACT_VIOLATION: do no check at all
// 
#if !(defined(LH_TERMINATE_ON_CONTRACT_VIOLATION ) ^ defined(LH_THROW_ON_CONTRACT_VIOLATION) ^ defined(LH_UNENFORCED_ON_CONTRACT_VIOLATION))
#define LH_THROW_ON_CONTRACT_VIOLATION
#endif

#if defined(LH_TERMINATE_ON_CONTRACT_VIOLATION)
namespace eds
{
	template <typename T>
	void Validates(const T& obj)
	{
		static_assert(!std::is_pointer<T>::value, "object to validate cannot be a pointer.");
		obj.__LH_CONTRACT_VALIDATES();
	}
}

#define InstanceContract() void __LH_CONTRACT_VALIDATES()

#define Asserts(cond) if(!(cond)) std::terminate();
#define Expects(cond) if(!(cond)) std::terminate();
#define Ensures(cond) if(!(cond)) std::terminate();

#define Validates(obj) eds::Validates(obj);

#elif defined(LH_THROW_ON_CONTRACT_VIOLATION)
namespace eds
{
	struct fast_fail : public std::runtime_error
	{
		explicit fast_fail(char const* const message) : std::runtime_error(message) { }
	};

	template <typename T>
	void Validates(const T& obj)
	{
		static_assert(!std::is_pointer<T>::value, "object to validate cannot be a pointer.");
		obj.__LH_CONTRACT_VALIDATES();
	}
}

#define InstanceContract() void __LH_CONTRACT_VALIDATES() const

#define Asserts(cond) if (!(cond)) \
    throw eds::fast_fail("lh: Assertion failure at " __FILE__ ": " LH_STRINGIFY(__LINE__));
#define Expects(cond) if (!(cond)) \
    throw eds::fast_fail("lh: Precondition failure at " __FILE__ ": " LH_STRINGIFY(__LINE__));
#define Ensures(cond) if (!(cond)) \
    throw eds::fast_fail("lh: Postcondition failure at " __FILE__ ": " LH_STRINGIFY(__LINE__));

#define Validates(obj) eds::Validates(obj);

#elif defined(LH_UNENFORCED_ON_CONTRACT_VIOLATION)

#define InstanceContract()

#define Asserts(cond)
#define Expects(cond)
#define Ensures(cond)
#defein Validates(obj)

#endif


#endif // !LH_CONTRACT_H

#ifndef LH_UTILITES_H
#define LH_UTILITES_H
namespace eds
{
    class Uncopyable
    {
    protected:
        Uncopyable()                                = default;
        Uncopyable(const Uncopyable &)              = delete;
        Uncopyable &operator =(const Uncopyable &)  = delete;
    };

    class Unmovable
    {
    protected:
        Unmovable()                                 = default;
        Unmovable(Unmovable &&)                     = delete;
        Unmovable &operator =(Unmovable &&)         = delete;
    };

	//
	// at() - Bounds-checked way of accessing static arrays, std::array, std::vector
	//
	template <typename T, size_t N>
	constexpr T& at(T(&arr)[N], size_t index)
	{
		Expects(index < N); return arr[index];
	}

	template <typename T, size_t N>
	constexpr T& at(std::array<T, N>& arr, size_t index)
	{
		Expects(index < N); return arr[index];
	}

	template <typename Container>
	constexpr typename Container::value_type& at(Container& cont, size_t index)
	{
		Expects(index < cont.size()); return cont[index];
	}

} // namespace eds

#endif // !LH_UTILITES_H

#ifndef LH_LANGHELPER_H
#define LH_LANGHELPER_H

// Owner<T*> indicate that this pointer do own the object and is responsible for allocation and delete
template <typename T>
using Owner = T;

// Access<T*> indicate that this pointer doesn't own the object
// so no delete is needed and allowed
// and usually it's not a good idea to store it unless context is obviously safe
template <typename T>
using Access = T;

template <typename T>
Owner<T> TransferOwner(Owner<T> &p)
{
    Owner<T> tmp = p;
    p = nullptr;
    
    return tmp;
}

#endif // !LH_LANGHELPER_H