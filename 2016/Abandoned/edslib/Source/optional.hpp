#pragma once
#include "lhelper.hpp"
#include "std_utility.hpp"
#include <type_traits>
#include <stdexcept>
#include <memory>

namespace eds
{
    ///<summary></summary>
    struct nullopt_t
    {
        // initialization token
        struct init_t { };

        // NOTE nullopt_t should be non DefaultConstructible
        // to eliminate anbiguity between operator=(optional&&) and operator=(nullopt_t)
        // when {} is assigned
        constexpr explicit nullopt_t(init_t) { }
    };
    static constexpr nullopt_t nullopt = nullopt_t{ nullopt_t::init_t{} };


    ///<summary>
    /// <c>bad_optional_access</c> is thrown when attempting to extract value from an empty optional object.
    ///</summary>
    class bad_optional_access : std::logic_error
    {
    public:
        bad_optional_access(const char *msg = "") : std::logic_error(msg) { }
    };

    namespace detail
    {
        //
        // Constexpr version
        //

        template <typename T>
        union ConstexprOptionalUnion
        {
            struct DummyType { };

            // ctor for null-state
            constexpr ConstexprOptionalUnion()
                : dummy_() { }
            // ctor for value
            template <typename ... TArgs>
            constexpr ConstexprOptionalUnion(DummyType, TArgs&& ...args)
                : value_(std::forward<TArgs>(args)...) { }

            // trivial dtor
            ~ConstexprOptionalUnion() = default;

            DummyType dummy_;
            T value_;
        };

        template <typename T>
        class ConstexprOptionalBase
        {
        public:
            // ctors
            constexpr ConstexprOptionalBase() noexcept
                : has_value_(false), buffer_() { }

            constexpr ConstexprOptionalBase(const T &other)
                : has_value_(true)
                , buffer_({}, other) { }
            constexpr ConstexprOptionalBase(T &&other)
                : has_value_(true)
                , buffer_({}, std::move(other)) { }

            template <typename ...TArgs>
            constexpr ConstexprOptionalBase(in_place_t, TArgs&& ...args)
                : has_value_(true)
                , buffer_({}, std::forward<TArgs>(args)...) { }

            // trivial dtor
            ~ConstexprOptionalBase() = default;

        public:
            // operations
            constexpr bool HasValue() const noexcept
            {
                return has_value_;
            }

            T &Extract() noexcept
            {
                // assume has_value_ == true
                return buffer_.value_;
            }
            constexpr const T &Extract() const noexcept
            {
                // assume has_value_ == true
                return buffer_.value_;
            }

            // construct a new instance at storage and assign has_value_ true
            template <typename ...TArgs>
            void Initialize(TArgs&& ...args) 
                noexcept(noexcept(T(std::forward<TArgs>(args)...)))
            {
                // assume has_value_ == false
                new (&buffer_.value_) T(std::forward<TArgs>(args)...);

                has_value_ = true;
            }

            void Clear() noexcept
            {
                // NOTE T is trivially destructible
                has_value_ = false;
            }

        private:
            bool has_value_;
            ConstexprOptionalUnion<T> buffer_;
        };

        //
        // Runtime version
        //

        template <typename T>
        union RuntimeOptionalUnion
        {
            struct DummyType { };

            // ctor for null-state
            constexpr RuntimeOptionalUnion(DummyType = {})
                : dummy_() { }
            // ctor for value
            template <typename ... TArgs>
            constexpr RuntimeOptionalUnion(DummyType, TArgs&& ...args)
                : value_(std::forward<TArgs>(args)...) { }

            // empty dtor
            ~RuntimeOptionalUnion() { };

            DummyType dummy_;
            T value_;
        };

        template <typename T>
        class RuntimeOptionalBase
        {
        public:
            // ctor
            constexpr RuntimeOptionalBase() noexcept
                : has_value_(false), buffer_() { }

            constexpr RuntimeOptionalBase(const T &other)
                : has_value_(true)
                , buffer_({}, other) { }
            constexpr RuntimeOptionalBase(T &&other)
                : has_value_(true)
                , buffer_({}, std::move(other)) { }

            template <typename ...TArgs>
            constexpr RuntimeOptionalBase(in_place_t, TArgs&& ...args)
                : has_value_(true)
                , buffer_({}, std::forward<TArgs>(args)...) { }

            // dtor
            ~RuntimeOptionalBase()
            {
                if (HasValue())
                {
                    Clear();
                }
            }

        public:
            // operations
            constexpr bool HasValue() const noexcept
            {
                return has_value_;
            }

            T &Extract() noexcept
            {
                // assume has_value_ == true
                return buffer_.value_;
            }
            constexpr const T &Extract() const noexcept
            {
                // assume has_value_ == true
                return buffer_.value_;
            }

            // construct a new instance at storage and assign has_value_ true
            template <typename ...TArgs>
            void Initialize(TArgs&& ...args)
                noexcept(noexcept(T(std::forward<TArgs>(args)...)))
            {
                // assume has_value_ == false
                new (std::addressof(buffer_.value_)) T(std::forward<TArgs>(args)...);
                has_value_ = true;
            }

            void Clear() noexcept
            {
                // assume has_value_ == true
                std::addressof<T>(buffer_.value_)->T::~T();
                has_value_ = false;
            }

        protected:
            bool has_value_;
            RuntimeOptionalUnion<T> buffer_;
        };

        template <typename T>
        using OptionalBase = std::conditional_t<
            std::is_trivially_destructible<T>::value,
            ConstexprOptionalBase<std::decay_t<T>>,
            RuntimeOptionalBase<std::decay_t<T>>>;

    } // namespace internal

    template <typename T>
    class optional : private detail::OptionalBase<T>
    {
    public:
        using value_type = T;
        using base_type = detail::OptionalBase<T>;

        static_assert(std::is_destructible<T>::value, "");
    public:
        // ctor
        constexpr optional() noexcept { }
        constexpr optional(nullopt_t) noexcept { }

        optional(const optional &other)
            noexcept(std::is_nothrow_copy_constructible<T>::value)
        {
            AssertCopyConstructible();
            if (other.has_value())
            {
                Initialize(other.Extract());
            }
        }
        optional(optional &&other)
            noexcept(std::is_nothrow_move_constructible<T>::value)
        {
            AssertMoveConstructible();
            if (other.has_value())
            {
                Initialize(std::move(other.Extract()));
            }
        }

        constexpr optional(const T &rhs)
            noexcept(std::is_nothrow_copy_constructible<T>::value)
            : base_type(rhs)
        {
            AssertCopyConstructible();
        }
        constexpr optional(T &&rhs)
            noexcept(std::is_nothrow_move_constructible<T>::value)
            : base_type(std::move(rhs))
        {
            AssertMoveConstructible();
        }

        template <typename ... TArgs>
        constexpr optional(in_place_t, TArgs&& ...args)
            : base_type(in_place, std::forward<TArgs>(args)...)
        {
        }

        optional &operator=(const optional &rhs)
        {
            AssertCopyConstructible();
            reset();

            if (rhs.has_value())
            {
                Initialize(rhs.Extract());
            }
            return *this;
        }

        optional &operator=(optional &&rhs)
        {
            AssertMoveConstructible();
            reset();

            if (rhs.has_value())
            {
                Initialize(std::move(rhs.Extract()));
            }
            return *this;
        }

        // dtor inherited from OptionalBase<T>
        ~optional() = default;

    public:
        ///<summary>Test if the optional instance has value in it.</summary>
        constexpr bool has_value() const noexcept
        {
            return HasValue();
        }

        value_type &value() &
        {
            TestHasValue();
            return Extract();
        }
        value_type &&value() &&
        {
            TestHasValue();
            return std::move(Extract());
        }
        constexpr const value_type &value() const &
        {
            TestHasValue();
            return Extract();
        }
        constexpr const value_type &&value() const &&
        {
            TestHasValue();
            return std::move(Extract());
        }

        template <typename U>
        const T value_or(U &&default_value) const &
        {
            AssertCopyConstructible();
            static_assert(std::is_convertible<U&&, T>::value, "");

            if (HasValue())
            {
                return Extract();
            }
            else
            {
                return static_cast<T>(std::forward<U>(default_value));
            }
        }
        template <typename U>
        const T value_or(U &&default_value) &&
        {
            AssertMoveConstructible();
            static_assert(std::is_convertible<U&&, T>::value, "");

            if (HasValue())
            {
                return std::move(Extract());
            }
            else
            {
                return static_cast<T>(std::forward<U>(default_value));
            }
        }

        void reset()
        {
            if (HasValue())
            {
                Clear();
            }
        }

        template <typename ...TArgs>
        void emplace(TArgs&& ...args)
        {
            reset(); // reset the instance anyway
            Initialize(std::forward<TArgs>(args)...);
        }
    public:
        T *operator->()
        {
            Asserts(HasValue());
            return &Extract();
        }
        const T *operator->() const
        {
            Asserts(HasValue());
            return &Extract();
        }

        T &operator*() &
        {
            Asserts(HasValue());
            return Extract();
        }
        T &&operator*() &&
        {
            Asserts(HasValue());
            return  std::move(Extract());
        }
        constexpr const T &operator*() const &
        {
            Asserts(HasValue());
            return Extract();
        }
        constexpr const T &&operator*() const &&
        {
            Asserts(HasValue());
            return std::move(Extract());
        }

        operator bool() const
        {
            return HasValue();
        }

    private:
        constexpr void AssertCopyConstructible() const
        {
            static_assert(std::is_copy_constructible<T>::value, "underlying type must be copy constructible.");
        }
        constexpr void AssertMoveConstructible() const
        {
            static_assert(std::is_move_constructible<T>::value, "underlying type must be move constructible.");
        }

        // throw bad_optional_access if has_value() returns false
        void TestHasValue()
        {
            if (!HasValue())
            {
                throw bad_optional_access{ "optional<T>::value: not engaged" };
            }
        }
    };

    // operators
    template <typename T>
    bool operator ==(const optional<T> &lhs, const optional<T> &rhs)
    {
        if (lhs)
        {
            return rhs && *lhs == *rhs;
        }
        else
        {
            return !rhs.has_value();
        }
    }
    template <typename T>
    bool operator ==(const optional<T> &lhs, nullopt_t)
    {
        return !lhs.has_value();
    }
    template <typename T>
    bool operator ==(nullopt_t, const optional<T> &rhs)
    {
        return !rhs.has_value();
    }

    template <typename T, typename ... TArgs>
    optional<T> make_optional(TArgs&& ...args)
    {
        return optional<T>(in_place, std::forward<TArgs>(args)...);
    }

} // namespace eds