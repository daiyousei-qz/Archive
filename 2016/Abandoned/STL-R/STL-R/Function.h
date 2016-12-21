#pragma once
#ifndef STLR_FUNCTION_H
#define STLR_FUNCTION_H

#include "Basic.h"

// not optimal implementation with Type Erasure
namespace eds
{
	namespace internal
	{
		// this is a space placeholder
		template <size_t N>
		struct Padding
		{
			uint8 Content[N];
		};


		// empty template declaration
		template <typename>
		class Function_Base;

		template <typename TRet, typename ... TArgs>
		class Function_Base<TRet(TArgs...)>
		{
		protected:
			using ReturnType = TRet;
			//using FunctionType = TRet(TArgs...);

			TRet Invoke(TArgs &&...args)
			{

			}

		private:

			Padding<12> _internal;
		};
	}

	// empty template declaration
	template <typename>
	class Function;

	template <typename TRet, typename ... TArgs>
	class Function<TRet(TArgs...)>
	{
	public:
		TRet operator()(TArgs &&...args)
		{
			//return Move(Invoke()
			throw 0;
		}
	};
}

#endif // !STLR_FUNCTION_H

// One possible version
/*
template<typename R, typename ...TArgs>
class ICallable : public Interface
{
public:
virtual R operator()(TArgs&& ...) = 0;
};

template<typename TFunc, typename R, typename ...TArgs>
class Functor : public ICallable<R, TArgs...>
{
TFunc _functor;

public:
Functor(TFunc f) : _functor(f) { }

R operator()(TArgs&& ...args) override
{
return _functor(Forward<TArgs>(args)...);
}
};

template<typename>
class Function;

template<typename R, typename... TArgs>
class Function<R(TArgs...)> : public ICallable<R, TArgs...>
{
ICallable<R, TArgs...> *_pf;

public:
template<typename TFunc>
Function(TFunc f) : _pf(new Functor<TFunc, R, TArgs...>(f)) { }
~Function() { delete _pf; }

R operator()(TArgs&& ...args)
{
return (*_pf)(Forward<TArgs>(args)...);
}
};
//*/