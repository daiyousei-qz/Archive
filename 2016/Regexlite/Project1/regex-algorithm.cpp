#pragma once
#include "regex-model.h"
#include "regex-automaton.h"
#include "regex-utility.h"
#include "unsafe_container.hpp"

namespace {

    using namespace eds;
    using namespace eds::regex;

    //
    // Visitor Base
    //
    using ExprTypeList = TypeList<
        ConcatenationExpr,
        AlternationExpr,
        RepetitionExpr,
        EntityExpr,
        AnchorExpr,
        CaptureExpr,
        ReferenceExpr,
        AssertionExpr>;

    template <typename TRet, typename ... TParams>
    class RegexAlgorithm
        : public Dispatcher<ExprBase, ExprTypeList, TRet, TParams...> { };

    //=================================================================================
    // DfaCompatibleAlgorithm

    // this algorithem test if the expression has used enhanced functionality, which requires more computation
    class CoreFunctionOnlyAlgorithm : public RegexAlgorithm<bool>
    {
    public:
        bool Apply(const ExprBase *expr)
        {
            return Dispatch(*expr);
        }

    protected:
        bool Visit(const ConcatenationExpr &expr) override
        {
            return IsCoreFunctional(expr);
        }
        bool Visit(const AlternationExpr &expr) override
        {
            return IsCoreFunctional(expr);
        }
        bool Visit(const RepetitionExpr &expr) override
        {
            return expr.Definition().strategy == ClosureStrategy::Greedy
                && Dispatch(*expr.UnderlyingExpr());
        }
        bool Visit(const EntityExpr &expr) override
        {
            return true;
        }
        bool Visit(const AnchorExpr &expr) override
        {
            return false;
        }
        bool Visit(const CaptureExpr &expr) override
        {
            return false;
        }
        bool Visit(const ReferenceExpr &expr) override
        {
            return false;
        }
        bool Visit(const AssertionExpr &expr) override
        {
            return false;
        }

    private:
        bool IsCoreFunctional(const ContainerExpr &expr)
        {
            auto children = expr.UnderlyingExprs();

            return std::all_of(children.FrontPointer(), children.BackPointer(), 
                [this](const ExprBase *child) 
            {
                return this->Dispatch(*child);
            });
        }
    };

    //=================================================================================
    // AssersionCompatibleAlgorithm

    // this algorithem test if the expression can be child of AssertionExpr
    class AssertionCompatibleAlgorithm : public CoreFunctionOnlyAlgorithm
    {
    protected:
        bool Visit(const RepetitionExpr &expr) override
        {
            return true;
        }
    };

    //=================================================================================
    // ConstructDictionaryAlgorithm & ApplyDictionaryAlgorithm

    // this algorithm generates a dictionary for remapping for the expression given
    // make sure TParams can be passed trvially
    template <typename ...TParams>
    class DictionaryAlgorithmBase : public RegexAlgorithm<void, TParams...>
    {
    protected:
        void Visit(const ConcatenationExpr &expr, TParams ...args) override 
        {
            VisitContainerExpr(expr, args...);
        }
        void Visit(const AlternationExpr &expr, TParams ...args) override 
        {
            VisitContainerExpr(expr, args...);
        }
        void Visit(const RepetitionExpr &expr, TParams ...args) override 
        {
            Dispatch(*expr.UnderlyingExpr(), args...);
        }
        
        void Visit(const AnchorExpr &expr, TParams ...) override 
        {
            ThrowIncompatibleError();
        }
        void Visit(const CaptureExpr &expr, TParams ...) override 
        {
            ThrowIncompatibleError();
        }
        void Visit(const ReferenceExpr &expr, TParams ...) override 
        {
            ThrowIncompatibleError();
        }
        void Visit(const AssertionExpr &expr, TParams ...) override 
        {
            ThrowIncompatibleError();
        }

    private:
        void ThrowIncompatibleError()
        {
            EvaluationAssert(false);
        }

        template <typename ...TArgs>
        void VisitContainerExpr(const ContainerExpr &expr, TArgs&& ...args)
        {
            auto children = expr.UnderlyingExprs();
            std::for_each(children.FrontPointer(), children.BackPointer(),
                [&](const ExprBase *child)
            {
                Dispatch(*child, std::forward<TArgs>(args)...);
            });
        }
    };

    // this algorithm rewrites the expression with a remaping dictionary generated from the algorithm above
    // and then yields the dictionary
    class ConstructDictionaryAlgorithm final : public DictionaryAlgorithmBase<RangeAccumulator&>
    {
    public:
        SymbolDictionary Apply(const ExprBase *expr)
        {
            RangeAccumulator accumulator;
            Dispatch(*expr, accumulator);

            return SymbolDictionary{ accumulator.ExtractDisjoint() };
        }

    protected:
        void Visit(const EntityExpr &expr, RangeAccumulator &accumulator) override
        {
            accumulator.Insert(expr.Definition());
        }
    };

    class RemapSymbolAlgorithm final : public DictionaryAlgorithmBase<const SymbolDictionary &>
    {
    public:
        SymbolDictionary Apply(const ExprBase *expr)
        {
            SymbolDictionary dict = ConstructDictionaryAlgorithm{}.Apply(expr);
            Dispatch(*expr, dict);

            return dict;
        }

    protected:
        void Visit(const EntityExpr &expr, const SymbolDictionary &dict) override
        {
            auto new_def = dict.Remap(expr.Definition());
            expr.Rewrite(new_def);
        }
    };

    //=================================================================================
    // BuildNfaAlgorithm

    // this algorithm generates a Nfa automaton in corespondance to the expression
    class BuildEpsilonNfaAlgorithm : public RegexAlgorithm<void, NfaBuilder&, NfaBranch>
    {
    public:
        BuildEpsilonNfaAlgorithm(bool right_to_left)
            : reversed_order_(right_to_left) { }

    public:
        NfaAutomaton Apply(const ExprBase *expr)
        {
            NfaBuilder builder;

            NfaBranch branch;
            branch.begin = builder.NewState();
            branch.end = builder.NewState();
            branch.end->is_final = true;

            Dispatch(*expr, builder, branch);

            return builder.Build(branch.begin);
        }

    protected:
        // visit connect NfaBranch given with the particular expr

        void Visit(const ConcatenationExpr &expr, NfaBuilder &builder, NfaBranch which) override
        {
            // which.begin - ... - ... - ... - which.end

            NfaState *begin = builder.NewState();
            NfaState *end = begin;

            auto visit_pred = 
                [&](const ExprBase *child)
            {
                NfaState *new_end = builder.NewState();
                this->Dispatch(*child, builder, NfaBranch{ end, new_end });

                end = new_end;
            };

            auto children = expr.UnderlyingExprs();
            if (!reversed_order_)
            {
                std::for_each(children.FrontPointer(), children.BackPointer(), visit_pred);
            }
            else
            {
                auto rbegin_iter = std::make_reverse_iterator(children.FrontPointer());
                auto rend_iter = std::make_reverse_iterator(children.BackPointer());
                std::for_each(rbegin_iter, rend_iter, visit_pred);
            }

            builder.NewEpsilonTransition(which.begin, begin, EpsilonPriority::Normal);
            builder.NewEpsilonTransition(end, which.end, EpsilonPriority::Normal);
        }
        void Visit(const AlternationExpr &expr, NfaBuilder &builder, NfaBranch which) override
        {
            //                ...
            //              /     \
            // which.begin -  ...  - which.end
            //              \     /
            //                ...

            auto children = expr.UnderlyingExprs();
            std::for_each(children.FrontPointer(), children.BackPointer(),
                [&](const ExprBase *child)
            {
                auto child_nfa = CreateNfa(*child, builder);

                builder.NewEpsilonTransition(which.begin, child_nfa.begin, EpsilonPriority::Normal);
                builder.NewEpsilonTransition(child_nfa.end, which.end, EpsilonPriority::Normal);
            });
        }
        void Visit(const RepetitionExpr &expr, NfaBuilder &builder, NfaBranch which) override
        {
            // evaluate child expression of repetition
            NfaBranch child_branch;
            child_branch.begin = builder.NewState();
            child_branch.end = builder.NewState();

            Dispatch(*expr.UnderlyingExpr(), builder, child_branch);

            // declarations
            Repetition closure = expr.Definition();

            std::vector<NfaState*> nodes;
            nodes.push_back(child_branch.begin);
            nodes.push_back(child_branch.end);
            for (size_t i = 1; i < closure.most; ++i)
            {
                // NOTE closure.kInfinity > closure.kMaxCount
                // so closure.most is always larger than closure.least

                if (i <= closure.least || closure.most != closure.kInifinity)
                {
                    NfaState *new_begin = nodes.back();
                    NfaState *new_end = builder.NewState();
                    builder.CloneBranch(new_begin, new_end, child_branch);

                    nodes.push_back(new_end);
                }
                else // closure.most == closure.kInfinity
                {
                    break;
                }
            }

            // Greedy closures tend to stay at internal state, while Reluctant closures behaves oppositely
            // forward_tendency is on epsilon transitions that tend to leave...
            EpsilonPriority forward_tendency, backward_tendency;
            if (closure.strategy == ClosureStrategy::Greedy)
            {
                forward_tendency = EpsilonPriority::Low;
                backward_tendency = EpsilonPriority::High;
            }
            else // closure.strategy == ClosureStrategy::Reluctant
            {
                forward_tendency = EpsilonPriority::High;
                backward_tendency = EpsilonPriority::Low;
            }

            if (closure.most != closure.kInifinity)
            {
                for (size_t i = closure.least; i < closure.most; ++i)
                {
                    builder.NewEpsilonTransition(nodes[i], nodes.back(), forward_tendency);
                }
            }
            else
            {
                NfaState *last_begin = nodes[nodes.size() - 2];
                NfaState *last_end = nodes.back();

                builder.NewEpsilonTransition(last_begin, last_end, forward_tendency);
                builder.NewEpsilonTransition(last_end, last_begin, backward_tendency);
            }

            builder.NewEpsilonTransition(which.begin, nodes.front(), EpsilonPriority::Normal);
            builder.NewEpsilonTransition(nodes.back(), which.end, forward_tendency);
        }
        void Visit(const EntityExpr &expr, NfaBuilder &builder, NfaBranch which) override
        {
            builder.NewEntityTransition(which.begin, which.end, expr.Definition());
        }
        void Visit(const AnchorExpr &expr, NfaBuilder &builder, NfaBranch which) override
        {
            builder.NewAnchorTransition(which.begin, which.end, expr.Definition());
        }
        void Visit(const CaptureExpr &expr, NfaBuilder &builder, NfaBranch which) override
        {
            auto child_nfa = CreateNfa(*expr.UnderlyingExpr(), builder);

            builder.NewCaptureTransition(which.begin, child_nfa.begin, expr.CaptureId());
            builder.NewFinishTransition(child_nfa.end, which.end);
        }
        void Visit(const ReferenceExpr &expr, NfaBuilder &builder, NfaBranch which) override
        {
            builder.NewReferenceTransition(which.begin, which.end, expr.CaptureId());
        }
        void Visit(const AssertionExpr &expr, NfaBuilder &builder, NfaBranch which) override
        {
            EvaluationAssert(AssertionCompatibleAlgorithm{}.Apply(expr.UnderlyingExpr()));

            AssertionType type = expr.Definition();
            bool lookbehind = type == AssertionType::PositiveLookBehind
                || type == AssertionType::NegativeLookBehind;

            NfaBranch child_nfa;
            if (lookbehind)
            {
                // NOTE
                ReverseScanningOrder();
                child_nfa = CreateNfa(*expr.UnderlyingExpr(), builder);
                ReverseScanningOrder();
            }
            else
            {
                child_nfa = CreateNfa(*expr.UnderlyingExpr(), builder);
            }

            builder.NewAssertionTransition(which.begin, child_nfa.begin, expr.Definition());
            builder.NewFinishTransition(child_nfa.end, which.end);
        }

    private:
        void ReverseScanningOrder()
        {
            reversed_order_ = !reversed_order_;
        }

        NfaBranch CreateNfa(const ExprBase &expr, NfaBuilder &builder)
        {
            NfaBranch result;
            result.begin = builder.NewState();
            result.end = builder.NewState();
            
            Dispatch(expr, builder, result);
            return result;
        }

    private:
        bool reversed_order_; // if ConcatenationExpr should be visited in reversed order
    };

} // namespace

namespace eds {
namespace regex {
    bool CoreFunctionOnlyInvoker(const ExprBase *root)
    {
        return CoreFunctionOnlyAlgorithm{}.Apply(root);
    }

    SymbolDictionary RewriteSymbolsInvoker(const ExprBase *root)
    {
        return RemapSymbolAlgorithm{}.Apply(root);
    }

    NfaAutomaton CreateEpsilonNfaInvoker(const ExprBase *root, bool right_to_left)
    {
        return BuildEpsilonNfaAlgorithm{ right_to_left }.Apply(root);
    }

} // namespace regex
} // namespace eds