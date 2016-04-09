#include <type_traits>

namespace detail
{
	template <size_t N>
	struct Padding
	{
		using uint8 = unsigned char;
		uint8 Content[N];

		template <typename T, typename ... TArgs>
		void Construct(TArgs&& ... args)
		{
			new (&Reinterpret<T>()) T(std::forward(args)...);
		}

		template <typename T>
		T &Reinterpret()
		{
			// such assert won't compile as instantiation is done without predication is not good
			// static_assert(sizeof(T) <= N, "Size of reinterpreted type cannot exceed the size of padding.");

			return *reinterpret_cast<T*>(&Content);
		}

		template <typename T>
		const T &Reinterpret() const
		{
			// such assert won't compile as instantiation is done without predication is not good
			//static_assert(sizeof(T) <= N, "Size of reinterpreted type cannot exceed the size of padding.");

			return *reinterpret_cast<const T*>(&Content);
		}
	};

	using FuncPadding = Padding<16>;

	struct FunctorTrait
	{
		const bool LocalFunctor;

		virtual void Destroy(FuncPadding &where) const = 0;
		virtual void Copy(FuncPadding &source, FuncPadding &target) const = 0;

	public:
		FunctorTrait(bool local) : LocalFunctor(local) { };
		FunctorTrait(const FunctorTrait &) = delete;
		FunctorTrait(FunctorTrait &&) = delete;
	};

	struct EmptyFunctorTrait : public virtual FunctorTrait
	{
		void Destroy(FuncPadding &) const override { }
		void Copy(FuncPadding &source, FuncPadding &target) const override { }

		EmptyFunctorTrait()
			: FunctorTrait(true) { }

		static EmptyFunctorTrait *GetInstance()
		{
			static EmptyFunctorTrait trait{};
			return &trait;
		}
	};

	template <typename TFunctor>
	struct LocalFunctorTrait : public virtual FunctorTrait
	{
		// you don't need to destroy a local functor
		void Destroy(FuncPadding &) const override { }

		void Copy(FuncPadding &source, FuncPadding &target) const override
		{
			auto &functor = source.Reinterpret<TFunctor>();
			// construct a new functor in-place
			new (&target.Reinterpret<TFunctor>()) TFunctor(functor);
		}

		LocalFunctorTrait()
			: FunctorTrait(true) { }

		static LocalFunctorTrait *GetInstance()
		{
			static LocalFunctorTrait trait{};
			return &trait;
		}
	};

	template <typename TFunctor>
	struct RemoteFunctorTrait : public virtual FunctorTrait
	{
		void Destroy(FuncPadding &where) const override
		{
			auto &functorPtr = where.Reinterpret<TFunctor*>();
			if (functorPtr != nullptr)
			{
				delete functorPtr;
				functorPtr = nullptr;
			}
		}

		void Copy(FuncPadding &source, FuncPadding &target) const override
		{
			auto &functorPtr = source.Reinterpret<TFunctor*>();
			target.Reinterpret<TFunctor*>() = new TFunctor(*functorPtr);
		}

		RemoteFunctorTrait()
			: FunctorTrait(false) { }

		static RemoteFunctorTrait *GetInstance()
		{
			static RemoteFunctorTrait trait{};
			return &trait;
		}
	};

	template <typename TRet, typename ... TArgs>
	using InvokerPtr = TRet (*)(FuncPadding &, TArgs&& ...);

	template <typename TRet, typename ... TArgs>
	TRet EmptyInvoker(FuncPadding &padding, TArgs&& ... args)
	{
		throw "empty function";
	}

	template <typename TFunctor, typename TRet, typename ... TArgs>
	TRet LocalInvoker(FuncPadding &padding, TArgs&& ... args)
	{
		using ResultType = decltype(std::declval<TFunctor>()(std::declval<TArgs>()...));
		static_assert(std::is_same<TRet, ResultType>::value, "");

		auto &functor = padding.Reinterpret<TFunctor>();
		return functor(std::forward(args)...);
	}

	template <typename TFunctor, typename TRet, typename ... TArgs>
	TRet RemoteInvoker(FuncPadding &padding, TArgs&& ... args)
	{
		using ResultType = decltype(std::declval<TFunctor>()(std::declval<TArgs>()...));
		static_assert(std::is_same<TRet, ResultType>::value, "");

		auto &functor = *padding.Reinterpret<TFunctor*>();
		return functor(std::forward(args)...);
	}

	template <typename TRet, typename ... TArgs>
	class Function_Impl
	{
	protected:
		Function_Impl()
		{
			InitializeEmpty();
		}

		Function_Impl(const Function_Impl &rhs)
		{
			_invoker = rhs._invoker;
			_trait = rhs._trait;

			if (!rhs._trait->LocalFunctor)
			{
				rhs._trait->Copy(rhs._padding, _padding);
			}
			else
			{
				_padding = rhs._padding;
			}
		}

		Function_Impl(Function_Impl &&rhs)
		{
			_invoker = rhs._invoker;
			_trait = rhs._trait;
			_padding = rhs._padding;

			rhs.InitializeEmpty();
		}

		Function_Impl(TRet(*pf)(TArgs...))
		{
			using FuncPtrType = TRet(*)(TArgs...);
			_invoker = &LocalInvoker<FuncPtrType, TRet, TArgs...>;
			_trait = LocalFunctorTrait<FuncPtrType>::GetInstance();

			_padding.Reinterpret<FuncPtrType>() = pf;
		}

		template <typename TFunctor>
		Function_Impl(const TFunctor &functor)
		{
			static_assert(std::is_copy_constructible<TFunctor>::value, "");

			if (sizeof(TFunctor) <= 16)
			{
				_invoker = &LocalInvoker<TFunctor, TRet, TArgs...>;
				_trait = LocalFunctorTrait<TFunctor>::GetInstance();

				// construct a new functor in-place
				new (&_padding.Reinterpret<TFunctor>()) TFunctor(functor);
			}
			else
			{
				_invoker = &RemoteInvoker<TFunctor, TRet, TArgs...>;
				_trait = RemoteFunctorTrait<TFunctor>::GetInstance();

				// construct a new functor in heap
				_padding.Reinterpret<TFunctor*>() = new TFunctor(functor);
			}
		}

		~Function_Impl()
		{
			Destroy();
		}

		void InitializeEmpty()
		{
			_invoker = &EmptyInvoker<TRet, TArgs...>;
			_trait = EmptyFunctorTrait::GetInstance();
		}

		bool IsEmpty()
		{
			return (_invoker == &EmptyInvoker<TRet, TArgs...>);
		}

		TRet Invoke(TArgs&& ... args)
		{
			return _invoker(_padding, std::forward(args)...);
		}

		void Destroy()
		{
			if (!_trait->LocalFunctor)
			{
				_trait->Destroy(_padding);
			}
		}

	private:
		InvokerPtr<TRet, TArgs...> _invoker;
		const FunctorTrait *_trait;
		FuncPadding _padding;
	};
}

template <typename T>
class Function;

template <typename TRet, typename ... TArgs>
class Function<TRet(TArgs...)> : private detail::Function_Impl<TRet, TArgs...>
{
	using BaseType = detail::Function_Impl<TRet, TArgs...>;

public:
	Function() : BaseType() { }
	Function(std::nullptr_t) : BaseType() { }

	Function(const Function &rhs) : BaseType(rhs) { }
	Function(Function &&rhs) : BaseType(rhs) { }

	Function(TRet(*pf)(TArgs...))
		: BaseType(pf) { }

	template <typename TFunctor>
	Function(const TFunctor &functor)
		: BaseType(functor) { }

	~Function()
	{
		Destroy();
	}

	operator bool()
	{
		return !IsEmpty();
	}

	TRet operator()(TArgs&& ... args)
	{
		return Invoke(std::forward(args)...);
	}
};