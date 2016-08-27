#pragma once
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

    ///<summary></summary>
    struct in_place_t { };
    static constexpr in_place_t in_place = {};

    ///<summary>
    /// <c>bad_optional_access</c> is thrown when attempting to extract value from an empty optional object.
    ///</summary>
    class bad_optional_access : std::logic_error
    {
    public:
        bad_optional_access(const char *msg = "") : std::logic_error(msg) { }
    };

    namespace internal
    {
        template <typename T>
        class TrivialOptionalBase
        {
        public:
            static_assert(std::is_trivially_destructible<T>::value, "!!!");

            // ctors
            constexpr TrivialOptionalBase() noexcept
                : has_value_(false), storage_() { }

            constexpr TrivialOptionalBase(const T &other)
                : has_value_(true)
                , storage_({}, other) { }
            constexpr TrivialOptionalBase(T &&other)
                : has_value_(true)
                , storage_({}, std::move(other)) { }

            template <typename ...TArgs>
            constexpr TrivialOptionalBase(in_place_t, TArgs&& ...args)
                : has_value_(true)
                , storage_({}, std::forward<TArgs>(args)...) { }

            // trivial dtor
            ~TrivialOptionalBase() = default;

        protected:
            union OptionalStorage
            {
                struct DummyType { };

                // ctor for null-state
                constexpr OptionalStorage()
                    : dummy_() { }
                // ctor for value
                template <typename ... TArgs>
                constexpr OptionalStorage(DummyType, TArgs&& ...args)
                    : value_(std::forward<TArgs>(args)...) { }

                // trivial dtor
                ~OptionalStorage() = default;

                DummyType dummy_;
                T value_;
            };

        protected:
            bool has_value_;
            OptionalStorage storage_;
        };

        template <typename T>
        class RegularOptionalBase
        {
        public:
            static_assert(!std::is_trivially_destructible<T>::value, "!!!");

            // ctor
            constexpr RegularOptionalBase() noexcept
                : has_value_(false), storage_() { }

            constexpr RegularOptionalBase(const T &other)
                : has_value_(true)
                , storage_({}, other) { }
            constexpr RegularOptionalBase(T &&other)
                : has_value_(true)
                , storage_({}, std::move(other)) { }

            template <typename ...TArgs>
            constexpr RegularOptionalBase(in_place_t, TArgs&& ...args)
                : has_value_(true)
                , storage_({}, std::forward<TArgs>(args)...) { }

            // trivial dtor
            ~RegularOptionalBase()
            {
                if (has_value_)
                {
                    storage_.value_.T::~T();
                }
            }

        protected:
            union OptionalStorage
            {
                struct DummyType { };

                // ctor for null-state
                constexpr OptionalStorage(DummyType = {})
                    : dummy_() { }
                // ctor for value
                template <typename ... TArgs>
                constexpr OptionalStorage(DummyType, TArgs&& ...args)
                    : value_(std::forward<TArgs>(args)...) { }

                // empty dtor
                ~OptionalStorage() { };

                DummyType dummy_;
                T value_;
            };

        protected:
            bool has_value_;
            OptionalStorage storage_;
        };

        template <typename T>
        using OptionalBase = std::conditional_t<
            std::is_trivially_destructible<T>::value,
            TrivialOptionalBase<std::decay_t<T>>,
            RegularOptionalBase<std::decay_t<T>>>;

    } // namespace internal

    template <typename T>
    class optional : private internal::OptionalBase<T>
    {
    public:
        using value_type = T;
        using base_type = internal::OptionalBase<T>;

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
            TestValue();
            return Extract();
        }
        value_type &&value() &&
        {
            TestValue();
            return std::move(Extract());
        }
        constexpr const value_type &value() const &
        {
            TestValue();
            return Extract();
        }
        constexpr const value_type &&value() const &&
        {
            TestValue();
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
            return &Extract();
        }
        const T *operator->() const
        {
            return &Extract();
        }

        T &operator*() &
        {
            return Extract();
        }
        T &&operator*() &&
        {
            return Extract();
        }
        constexpr const T &operator*() const &
        {
            return Extract();
        }
        constexpr const T &&operator*() const &&
        {
            return Extract();
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
        void TestValue()
        {
            if (!HasValue())
            {
                throw bad_optional_access{ "optional<T>::value: not engaged" };
            }
        }

        constexpr bool HasValue() const noexcept
        {
            return has_value_;
        }

        T &Extract() noexcept
        {
            // assume has_value_ == true
            return storage_.value_;
        }
        constexpr const T &Extract() const noexcept
        {
            // assume has_value_ == true
            return storage_.value_;
        }

        // construct a new instance at storage and assign has_value_ true
        template <typename ...TArgs>
        void Initialize(TArgs&& ...args)
        {
            // assume has_value_ == false
            new (&storage_.value_) T(std::forward<TArgs>(args)...);

            has_value_ = true;
        }

        // destroy the instance and assign has_value_ false
        void Clear() noexcept
        {
            // assume has_value_ == true
            storage_.value_.T::~T();

            has_value_ = false;
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