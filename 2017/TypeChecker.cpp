#pragma once
#include <type_traits>

// A sample implementation of concepts mentioned in https://www.youtube.com/watch?v=PFdWqa68LmA
namespace eds::type
{
	namespace detail
	{
		// A checker type should inherts from TypeChecker
		// and implement eval<T>()
		struct TypeChecker {};

		template <typename ...Ps>
		inline constexpr bool IsTypeChecker()
		{
			return std::conjunction_v<
				std::is_base_of<TypeChecker, Ps>...
			>;
		}

		template <typename ...Ps>
		struct EnsureTypeChecker
			: std::enable_if_t<IsTypeChecker<Ps...>()> { };

		// Basic Compositions
		//

		template <typename P1, typename P2>
		struct And : public TypeChecker
		{
			template <typename T>
			static constexpr bool eval()
			{
				return P1::template eval<T>()
					&& P2::template eval<T>();
			}
		};

		template <typename P1, typename P2>
		struct Or : public TypeChecker
		{
			template <typename T>
			static constexpr bool eval()
			{
				return P1::template eval<T>()
					|| P2::template eval<T>();
			}
		};

		template <typename P>
		struct Not : public TypeChecker
		{
			template <typename T>
			static constexpr bool eval()
			{
				return !P::template eval<T>();
			}
		};

		// TypeChecker Impl
		//

		template <template <typename> Pred>
		struct StlTypeChecker : public TypeChecker
		{
			template <typename T>
			static constexpr bool eval()
			{
				return Pred<T>::value;
			}
		};

		// Primary type categories
		//

		using IsVoid								= StlTypeChecker<std::is_void>;
		using IsNullPointer							= StlTypeChecker<std::is_null_pointer>;
		using IsIntegral							= StlTypeChecker<std::is_integral>;
		using IsFloatingPoint						= StlTypeChecker<std::is_floating_point>;
		using IsArray								= StlTypeChecker<std::is_array>;
		using IsEnum								= StlTypeChecker<std::is_enum>;
		using IsUnion								= StlTypeChecker<std::is_floating_point>;
		using IsClass								= StlTypeChecker<std::is_class>;
		using IsFunction							= StlTypeChecker<std::is_function>;
		using IsPointer								= StlTypeChecker<std::is_pointer>;
		using IsLvalueReference						= StlTypeChecker<std::is_lvalue_reference>;
		using IsRvalueReference						= StlTypeChecker<std::is_rvalue_reference>;
		using IsMemberObjectPointer					= StlTypeChecker<std::is_member_object_pointer>;
		using IsMemberFunctionPointer				= StlTypeChecker<std::is_member_function_pointer>;

		// Composite type categories
		//

		using IsFundamental							= StlTypeChecker<std::is_fundamental>;
		using IsArithmetic							= StlTypeChecker<std::is_arithmetic>;
		using IsScalar								= StlTypeChecker<std::is_scalar>;
		using IsObject								= StlTypeChecker<std::is_object>;
		using IsCompound							= StlTypeChecker<std::is_compound>;
		using IsReference							= StlTypeChecker<std::is_reference>;
		using IsMemberPointer						= StlTypeChecker<std::is_member_pointer>;

		// Type properties
		//

		using IsConst								= StlTypeChecker<std::is_const>;
		using IsVolatile							= StlTypeChecker<std::is_volatile>;
		using IsTrivial								= StlTypeChecker<std::is_trivial>;
		using IsTriviallyCopyable					= StlTypeChecker<std::is_trivially_copyable>;
		using IsStandardLayout						= StlTypeChecker<std::is_standard_layout>;
		using IsPod									= StlTypeChecker<std::is_pod>;
		using IsEmpty								= StlTypeChecker<std::is_empty>;
		using IsPolymorphic							= StlTypeChecker<std::is_polymorphic>;
		using IsAbstract							= StlTypeChecker<std::is_abstract>;
		using IsFinal								= StlTypeChecker<std::is_final>;
		using IsSigned								= StlTypeChecker<std::is_signed>;
		using IsUnsigned							= StlTypeChecker<std::is_unsigned>;

		// Type relationships
		//

		template <typename U>
		struct SameTo : public TypeChecker
		{
			template <typename T>
			static constexpr bool eval()
			{
				return std::is_same_v<U, T>;
			}
		};

		template <typename Base>
		struct DeriveFrom : public TypeChecker
		{
			template <typename T>
			static constexpr bool eval()
			{
				return std::is_base_of_v<Base, T>;
			}
		};

		template <typename U>
		struct ConvertibleTo : public TypeChecker
		{
			template <typename T>
			static constexpr bool eval()
			{
				return std::is_convertible_v<T, U>;
			}
		};

		// Extensions
		//

		template <typename U>
		struct SimilarTo : public TypeChecker
		{
			template <typename T>
			static constexpr bool eval()
			{
				return std::is_same_v<std::decay_t<U>, std::decay_t<T>>;
			}
		};
	}

	// Operator overloads
	//

	template <
		typename P1,
		typename P2,
		typename = detail::EnsureTypeChecker<P1, P2>>
	inline constexpr auto operator&&(P1, P2)
	{
		return detail::And<P1, P2>();
	}

	template <
		typename P1,
		typename P2,
		typename = detail::EnsureTypeChecker<P1, P2>>
	inline constexpr auto operator||(P1, P2)
	{
		return detail::Or<P1, P2>();
	}

	template <
		typename P,
		typename = detail::EnsureTypeChecker<P>>
	inline constexpr auto operator!(P)
	{
		return detail::Not<P>();
	}

	// TypeChecker instances
	//

	static constexpr auto is_void						= detail::IsVoid();
	static constexpr auto is_null_pointer				= detail::IsNullPointer();
	static constexpr auto is_integral					= detail::IsIntegral();
	static constexpr auto is_floating_point				= detail::IsFloatingPoint();
	static constexpr auto is_array						= detail::IsArray();
	static constexpr auto is_enum						= detail::IsEnum();
	static constexpr auto is_union						= detail::IsUnion();
	static constexpr auto is_class						= detail::IsClass();
	static constexpr auto is_function					= detail::IsFunction();
	static constexpr auto is_pointer					= detail::IsPointer();
	static constexpr auto is_lvalue_reference			= detail::IsLvalueReference();
	static constexpr auto is_rvalue_reference			= detail::IsRvalueReference();
	static constexpr auto is_member_object_pointer		= detail::IsMemberObjectPointer();
	static constexpr auto is_member_function_pointer	= detail::IsMemberFunctionPointer();

	static constexpr auto is_fundamental				= detail::IsFundamental();
	static constexpr auto is_arithmetic					= detail::IsArithmetic();
	static constexpr auto is_scalar						= detail::IsScalar();
	static constexpr auto is_object						= detail::IsObject();
	static constexpr auto is_compound					= detail::IsCompound();
	static constexpr auto is_reference					= detail::IsReference();
	static constexpr auto is_member_pointer				= detail::IsMemberPointer();

	static constexpr auto is_const						= detail::IsConst();
	static constexpr auto is_volatile					= detail::IsVolatile();
	static constexpr auto is_trivial					= detail::IsTrivial();
	static constexpr auto is_trivially_copyable			= detail::IsTriviallyCopyable();
	static constexpr auto is_standard_layout			= detail::IsStandardLayout();
	static constexpr auto is_pod						= detail::IsPod();
	static constexpr auto is_empty						= detail::IsEmpty();
	static constexpr auto is_polymorphic				= detail::IsPolymorphic();
	static constexpr auto is_abstract					= detail::IsAbstract();
	static constexpr auto is_final						= detail::IsFinal();
	static constexpr auto is_signed						= detail::IsSigned();
	static constexpr auto is_unsigned					= detail::IsUnsigned();

	template <typename U>
	static constexpr auto same_to						= detail::SameTo<U>();
	template <typename Base>
	static constexpr auto derive_from					= detail::DeriveFrom<Base>();
	template <typename U>
	static constexpr auto convertible_to				= detail::ConvertibleTo<U>();

	template <typename U>
	static constexpr auto similar_to					= detail::SimilarTo<U>();

	// Evaluation functions
	//

	template <
		typename T,
		typename P,
		typename = detail::EnsureTypeChecker<P>>
	static constexpr bool Constraint(P)
	{
		return P::template eval<T>();
	}
}