#pragma once
#include "type_utils.hpp"
#include <stdexcept>

namespace eds
{
    class BadDispatchError : std::runtime_error
    {
    public:
        BadDispatchError(const char *msg)
            : std::runtime_error(msg) { }
    };

    namespace detail
    {
        template <typename TBase, typename DerivedTypeList, typename TRet, typename ...TParams>
        class DispatcherBase
            : public DispatcherBase<TBase, TypeListOps::NextList<DerivedTypeList>, TRet, TParams...>
        {
            using MyBaseType = DispatcherBase<TBase, TypeListOps::NextList<DerivedTypeList>, TRet, TParams...>;
            using MyDispatchedType = TypeListOps::FrontType<DerivedTypeList>;

        protected:
            virtual TRet Visit(const MyDispatchedType &, TParams...) = 0;

            template <typename ...TArgs>
            inline TRet DispatchInternal(const std::type_info &info, const TBase &obj, TArgs&& ...args)
            {
                // compare pointer to type_info for efficiency
                // if Dispatcher is used in shared context, this approach is unsafe
                if (&info == &typeid(MyDispatchedType))
                {
                    return Visit(reinterpret_cast<const MyDispatchedType &>(obj), std::forward<TArgs>(args)...);
                }

                return MyBaseType::DispatchInternal(info, obj, std::forward<TArgs>(args)...);
            }
        };

        template <typename TBase, typename TRet, typename ...TParams>
        class DispatcherBase<typename TBase, TypeList<>, TRet, TParams...>
        {
        protected:
            template <typename ...TArgs>
            TRet DispatchInternal(const std::type_info &info, const TBase &obj, TArgs&& ...args)
            {
                throw BadDispatchError("failed to dispatch this object");
            }
        };

        template <typename TBase, typename TDerivedList>
        struct CanBeDispatchedHelper;
        template <typename TBase, typename ...TDerived>
        struct CanBeDispatchedHelper<TBase, TypeList<TDerived...>>
            : std::bool_constant<
                std::is_polymorphic<TBase>::value
                && std::conjunction<std::is_base_of<TBase, TDerived>...>::value> { };
    }

    template <typename TBase,
        typename DerivedTypeList,
        typename TRet,
        typename ...TParams>
        class Dispatcher
        : public detail::DispatcherBase<TBase, DerivedTypeList, TRet, TParams...>
    {
    private:
        using MyBaseType = detail::DispatcherBase<TBase, DerivedTypeList, TRet, TParams...>;

        static_assert(detail::CanBeDispatchedHelper<TBase, DerivedTypeList>::value, "!!!");

    public:
        template <typename ...TArgs>
        TRet Dispatch(const TBase &obj, TArgs&& ...args)
        {
            return MyBaseType::DispatchInternal(typeid(obj), obj, std::forward<TArgs>(args)...);
        }
    };
}