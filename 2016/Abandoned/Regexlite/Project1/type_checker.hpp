#pragma once
#include <type_traits>

namespace eds
{
    //============================================================================================
    // Type Checker

    /*
    * TypeChecker is a wrapper struct for type traits test
    *
    * A standard type checker is a struct with a static constexpr function in signature of
    * "bool Test<TInput>()", it takes a type TInput as its only parameter and returns a bool
    * to indicate if type of TInput is ...
    */

    namespace type_checker
    {
        template <typename TCompare>
        struct IsSame
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_same<TCompare, TInput>::value;
            }
        };

        template <typename TBase>
        struct DerivedFrom
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_base_of<TBase, TInput>::value;
            }
        };


        //
        // Primary type categories
        //
        struct IsVoid
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_void<TInput>::value;
            }
        };
        struct IsNullPointer
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_null_pointer<TInput>::value;
            }
        };
        struct IsIntegral
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_integral<TInput>::value;
            }
        };
        struct IsFloatingPoint
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_floating_point<TInput>::value;
            }
        };
        struct IsArray
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_array<TInput>::value;
            }
        };
        struct IsEnum
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_enum<TInput>::value;
            }
        };
        struct IsUnion
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_union<TInput>::value;
            }
        };
        struct IsClass
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_class<TInput>::value;
            }
        };
        struct IsFunction
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_function<TInput>::value;
            }
        };
        struct IsPointer
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_pointer<TInput>::value;
            }
        };
        struct IsLvalueReference
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_lvalue_reference<TInput>::value;
            }
        };
        struct IsRvalueReference
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_rvalue_reference<TInput>::value;
            }
        };
        struct IsMemberObjectPointer
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_member_object_pointer<TInput>::value;
            }
        };
        struct IsMemberFunctionPointer
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_member_function_pointer<TInput>::value;
            }
        };

        //
        // Composite type categories
        //
        struct IsFundamental
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_fundamental<TInput>::value;
            }
        };
        struct IsArithmetic
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_arithmetic<TInput>::value;
            }
        };
        struct IsScalar
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_scalar<TInput>::value;
            }
        };
        struct IsObject
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_object<TInput>::value;
            }
        };
        struct IsCompound
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_compound<TInput>::value;
            }
        };
        struct IsReference
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_reference<TInput>::value;
            }
        };
        struct IsMemberPointer
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_member_pointer<TInput>::value;
            }
        };

        //
        // Type properties
        //
        struct IsConst
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_const<TInput>::value;
            }
        };
        struct IsVolatile
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_volatile<TInput>::value;
            }
        };
        struct IsTrivial
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_trivial<TInput>::value;
            }
        };
        struct IsTriviallyCopyable
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_trivially_copyable<TInput>::value;
            }
        };
        struct IsPOD
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_pod<TInput>::value;
            }
        };
        struct IsEmpty
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_empty<TInput>::value;
            }
        };
        struct IsPolymorphic
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_polymorphic<TInput>::value;
            }
        };
        struct IsFinal
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_final<TInput>::value;
            }
        };
        struct IsAbstract
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_abstract<TInput>::value;
            }
        };
        struct IsSigned
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_signed<TInput>::value;
            }
        };
        struct IsUnsigned
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_unsigned<TInput>::value;
            }
        };

        //
        // Supported operations 
        //
        
        struct IsDefaultConstructible
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_default_constructible<TInput>::value;
            }
        };
        struct IsTriviallyDefaultConstructible
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_trivially_default_constructible<TInput>::value;
            }
        };
        struct IsNothrowDefaultConstructible
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_nothrow_default_constructible<TInput>::value;
            }
        };

        struct IsCopyConstructible
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_copy_constructible<TInput>::value;
            }
        };
        struct IsTriviallyCopyConstructible
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_trivially_copy_constructible<TInput>::value;
            }
        };
        struct IsNothrowCopyConstructible
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_nothrow_copy_constructible<TInput>::value;
            }
        };

        struct IsMoveConstructible
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_move_constructible<TInput>::value;
            }
        };
        struct IsTriviallyMoveConstructible
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_trivially_move_constructible<TInput>::value;
            }
        };
        struct IsNothrowMoveConstructible
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_nothrow_move_constructible<TInput>::value;
            }
        };

        struct IsCopyAssignable
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_copy_assignable<TInput>::value;
            }
        };
        struct IsTriviallyCopyAssignable
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_trivially_copy_assignable<TInput>::value;
            }
        };
        struct IsNothrowCopyAssignable
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_nothrow_copy_assignable<TInput>::value;
            }
        };

        struct IsMoveAssignable
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_move_assignable<TInput>::value;
            }
        };
        struct IsTriviallyMoveAssignable
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_trivially_move_assignable<TInput>::value;
            }
        };
        struct IsNothrowMoveAssignable
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_nothrow_move_assignable<TInput>::value;
            }
        };

        struct IsDestructible
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_destructible<TInput>::value;
            }
        };
        struct IsTriviallyDestructible
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_trivially_destructible<TInput>::value;
            }
        };
        struct IsNothrowDestructible
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::is_nothrow_destructible<TInput>::value;
            }
        };

        struct HasVirtualDestructor
        {
            template <typename TInput>
            static constexpr bool Test()
            {
                return std::has_virtual_destructor<TInput>::value;
            }
        };
    }
}