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

			template <typename T>
			T &Reinterpret()
			{
				static_assert(sizeof(T) <= N, "Size of reinterpreted type cannot exceed the size of padding.");

				return *reinterpret_cast<T*>(Content);
			}
		};

		template <typename TFunc>
		struct FunctionMetadata
		{
			static constexpr bool IsFunctionPointer = false;
			static constexpr size_t FunctorSize = 4 + sizeof(TFunc); // v-table + sizeof(TFunc)
		};

		template <typename TFunc>
		struct FunctionMetadata<typename std::enable_if<
									std::is_pointer<TFunc>::value && 
										!std::is_class<typename std::remove_pointer<TFunc>::type>::value, 
									TFunc>::type>
		{
			static constexpr bool IsFunctionPointer = true;
		};

		template <typename TRet, typename ... TArgs>
		class FunctionInvoker
		{
		public:
			virtual TRet Invoke(TArgs &&...args) = 0;
		};

		template <typename TFunc, typename TRet, typename ... TArgs>
		class Functor
		{
			TFunc _delegate;
		public:
			Functor(const TFunc &func)
				: _delegate(func) { }

			TRet Invoke(TArgs &&...args) override
			{
				return std::move(_delegate(std::forward(args)...));
			}
		};

		template <typename TRet, typename ... TArgs>
		class Function_Base
		{
		protected:
			using ReturnType = TRet;
			using FunctionPointerType = TRet(*)(TArgs...);
			using InvokerType = FunctionInvoker<TRet, TArgs...>;

			template <typename TFunc>
			Function_Base(const TFunc &func)
			{
				using FunctorType = Functor<TFunc, TRet, TArgs...>;

				if (FunctionMetadata<TFunc>::IsFunctionPointer)
				{
					_type = PointerToFunction;
					_buffer.Reinterpret<FunctionPointerType>() = func;
				}
				else if (FunctionMetadata<TFunc>::FunctorSize <= 12)
				{
					_type = InPlaceFunctor;
					_buffer.Reinterpret<FunctorType> = func;
				}
				else
				{
					_type = PointerToFunctor;
					_buffer.Reinterpret<InvokerType*> = new FunctorType(func);
				}
			}

			TRet Invoke(TArgs &&...args)
			{
				if (_type == PointerToFunction)
				{
					const FunctionPointerType &pfunc = _buffer.Reinterpret<FunctionPointerType>();
					return std::move(pfunc(std::forward(args)...));
				}
				else if (_type == InPlaceFunctor)
				{
					const InvokerType *pcallable = &_buffer.Reinterpret<InvokerType>();
					return std::move(pcallable(std::forward(args)...));
				}
				else if (_type == PointerToFunctor)
				{
					const InvokerType *pcallable = _buffer.Reinterpret<InvokerType*>();
					return std::move(pcallable(std::forward(args)...));
				}
			}

			void Destroy()
			{
				if (_type == PointerToFunctor)
				{
					delete _buffer.Reinterpret<InvokerType*>();
				}
			}

		private:

			enum FunctorType
			{
				PointerToFunction,
				InPlaceFunctor,
				PointerToFunctor
			} _type;
			Padding<12> _buffer;
		};
	}

	// empty template declaration
	template <typename>
	class Function;

	template <typename TRet, typename ... TArgs>
	class Function<TRet(TArgs...)> : internal::Function_Base<TRet, TArgs...>
	{
	public:
		template <typename TFunc>
		Function(const TFunc &func)
			: internal::Function_Base<TRet, TArgs...>(std::forward(func)) { }
		~Function()
		{
			Destroy();
		}

		TRet operator()(TArgs &&...args)
		{
			return std::move(BaseType::Invoke(std::forward(args)...));
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