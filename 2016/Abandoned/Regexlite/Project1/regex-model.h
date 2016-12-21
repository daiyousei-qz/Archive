#pragma once
#include "unsafe_container.hpp"
#include "dispatcher.hpp"
#include <cstddef>
#include <numeric>

namespace eds {
namespace regex {

    //=================================================================================
    // Basic Definitions

    // symbol_t and capture_t are assumed to be an unsigned integer
    using symbol_t = std::uint32_t;
    using capture_t = std::uint32_t;

    static constexpr capture_t kInvalidCaptureId = std::numeric_limits<symbol_t>::max();
    static constexpr symbol_t kInvalidSymbol = std::numeric_limits<symbol_t>::max();

    enum class EpsilonPriority
    {
        Low,
        Normal,
        High,
    };

    enum class AnchorType
    {
        Circumflex,
        Dollar,
    };

    enum class AssertionType
    {
        PositiveLookAhead,
        NegativeLookAhead,
        PositiveLookBehind,
        NegativeLookBehind
    };

    enum class ClosureStrategy
    {
        Greedy,
        Reluctant,
    };

    // A interval of symbols of [min, max)
    struct SymbolRange
    {
    public:
        symbol_t min;
        symbol_t max;

    public:
        SymbolRange(symbol_t left, symbol_t right)
            : min(left), max(right)
        {
            Ensures(Validate());
        }
        SymbolRange(symbol_t ch)
            : SymbolRange(ch, ch + 1) { }

        size_t Length() const noexcept
        {
            return max - min;
        }

        bool Contain(symbol_t value) const noexcept
        {
            return value >= min && value < max;
        }

        bool Contain(SymbolRange range) const noexcept
        {
            return range.min >= min && range.max <= max;
        }

        bool Validate() const noexcept
        {
            return min < max && !Contain(kInvalidSymbol);
        }
    };

    struct Repetition
    {
    public:
        using CountType = size_t;

        static constexpr CountType kMaxCount = 2048;
        static constexpr CountType kInifinity = kMaxCount + 1;
    public:
        Repetition(CountType least_count, CountType most_count, ClosureStrategy s)
            : least(least_count), most(most_count), strategy(s)
        {
            Validate();
        }

        void Validate()
        {
            Asserts(least >= 0);
            Asserts(most >= least);
            Asserts(most <= kInifinity);
            Asserts(!(least == 0 && most == 0));
        }

        CountType least, most;
        ClosureStrategy strategy;
    };

    //=================================================================================
    // Expr Model

    class ExprBase
    {
    protected:
        ExprBase() = default;
        ~ExprBase() = default;

    private:
        // to make sure vtable is generated
        virtual void PlaceholderFunc() { }
    };

    class WrapperExpr : public ExprBase
    {
    public:
        WrapperExpr(ExprBase *p)
            : child_(p) { }

        ExprBase *UnderlyingExpr() const
        {
            return child_;
        }
    private:
        ExprBase *child_;
    };

    class ContainerExpr : public ExprBase
    {
    public:
        ContainerExpr(ArrayRef<ExprBase*> content)
            : children_(content) { }

        const ArrayRef<ExprBase*> UnderlyingExprs() const
        {
            return children_;
        }
    private:
        ArrayRef<ExprBase*> children_;
    };

    //
    // Definitions
    //

    class ConcatenationExpr : public ContainerExpr
    {
    public:
        ConcatenationExpr(ArrayRef<ExprBase*> content)
            : ContainerExpr(content) { }
    };

    class AlternationExpr : public ContainerExpr
    {
    public:
        AlternationExpr(ArrayRef<ExprBase*> content)
            : ContainerExpr(content) { }
    };

    class RepetitionExpr : public WrapperExpr
    {
    public:
        RepetitionExpr(ExprBase *child, Repetition def)
            : WrapperExpr(child) , def_(def) { }

        Repetition Definition() const
        {
            return def_;
        }

    private:
        Repetition def_;
    };

    class EntityExpr : public ExprBase
    {
    public:
        EntityExpr(SymbolRange range)
            : def_(range) { }

        void Rewrite(SymbolRange range) const
        {
            def_ = range;
        }

        SymbolRange Definition() const
        {
            return def_;
        }
    private:
        mutable SymbolRange def_;
    };

    class AnchorExpr : public ExprBase
    {
    public:
        AnchorExpr(AnchorType type)
            : def_(type) { }

        AnchorType Definition() const
        {
            return def_;
        }
    private:
        AnchorType def_;
    };

    class CaptureExpr : public WrapperExpr
    {
    public:
        CaptureExpr(ExprBase *p, capture_t id)
            : WrapperExpr(p), id_(id) { }

        capture_t CaptureId() const
        {
            return id_;
        }
    private:
        capture_t id_;
    };

    class ReferenceExpr : public ExprBase
    {
    public:
        ReferenceExpr(capture_t id)
            : id_(id) { }

        capture_t CaptureId() const
        {
            return id_;
        }
    private:
        capture_t id_;
    };

    class AssertionExpr : public WrapperExpr
    {
    public:
        AssertionExpr(ExprBase *p, AssertionType def)
            : WrapperExpr(p), type_(def) { }

        AssertionType Definition() const
        {
            return type_;
        }

    private:
        AssertionType type_;
    };

    //=================================================================================
    // Automaton Model

    struct NfaTransition;
    struct NfaState;

    enum class TransitionType
    {
        Epsilon,    // empty transition
        Entity,     // for codepoints
        Anchor,     // builtin zero-width assertion
        Capture,    // capture
        Reference,  // backreference
        Assertion,  // custom zero-width assertion
        Finish,     // end mark for Capture, Reference and Assertion
    };

    struct NfaTransition
    {
        const NfaState *source;
        const NfaState *target;
        TransitionType type;

        union
        {
            EpsilonPriority     priority;     // valid only when type is Epsilon
            AnchorType          anchor;       // valid only when type is Anchor
            SymbolRange         values;       // valid only when type is Entity
            capture_t           capture_id;   // valid only when type is Capture or Reference
            AssertionType       assertion;    // valid only when type is Assertion
        } data;
    };

    struct NfaState
    {
        bool is_final;                              // indicate whether this state is accepting
        bool is_checkpoint;                         // indicate whether this state should be backtracked
        VectorAdapter<const NfaTransition*> exits;  // where outgoing edges stores
    };

    struct NfaBranch
    {
        NfaState *begin;
        NfaState *end;
    };
} // namespace regex
} // namespace eds