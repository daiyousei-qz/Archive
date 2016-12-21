#pragma once
#include "regex-model.h"
#include "regex-symbol.h"
#include "arena.hpp"
#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>

namespace eds {
namespace regex {

    class DfaAutomaton
    {
    public:
        using StateType = int;

    private:
        friend class DfaBuilder;
        DfaAutomaton(std::unordered_set<StateType> accepting_states,
                     std::vector<StateType> jumptable,
                     SymbolDictionary dict)
            : accepting_states_(std::move(accepting_states))
            , jumptable_(std::move(jumptable))
            , dict_(std::move(dict))
        { 
            state_cnt_ = jumptable_.size() / dict_.LexemeCount();
        }

    public:
        static constexpr StateType InvalidState()
        {
            return -1;
        }

        static constexpr StateType IntialState()
        {
            return 0;
        }
    public:
        bool IsAccepting(StateType state) const
        {
            return accepting_states_.find(state) != accepting_states_.end();
        }
        int Transit(StateType source_state, int codepoint) const
        {
            TestStateInput(source_state);
            
            int symbol = dict_.Translate(codepoint);
            if (symbol != kInvalidSymbol)
            {
                return jumptable_[source_state * dict_.LexemeCount() + symbol];
            }
            else
            {
                return -1;
            }
        }
    private:
        void TestStateInput(StateType s) const
        {
            Asserts(s == InvalidState() || (s >= 0 && s < state_cnt_));
        }

    private:
        int state_cnt_;

        std::unordered_set<StateType> accepting_states_;
        std::vector<StateType> jumptable_;
        SymbolDictionary dict_;
    };

    class DfaBuilder
    {
    public:
        DfaBuilder(SymbolDictionary dict)
            : next_id_(0), dict_(std::move(dict))
        { }

    public:
        int NewState(bool accepting)
        {
            jumptable_.resize(jumptable_.size() + dict_.LexemeCount(), -1);
            int id = next_id_++;

            if (accepting)
            {
                accepting_.insert(id);
            }

            return id;
        }

        void NewTransition(int source_state, int target_state, size_t symbol)
        {
            Expects(source_state < next_id_);
            Expects(target_state < next_id_);
            Expects(symbol < dict_.LexemeCount());

            jumptable_[source_state * dict_.LexemeCount() + symbol] = target_state;
        }

        DfaAutomaton Build()
        {
            return DfaAutomaton{ std::move(accepting_), std::move(jumptable_), std::move(dict_) };
        }
    private:
        int next_id_;
        std::unordered_set<int> accepting_;
        std::vector<int> jumptable_;
        SymbolDictionary dict_;
    };

    class NfaAutomaton
    {
    private:
        friend class NfaBuilder;
        NfaAutomaton(Arena guard, NfaState *begin)
            : arena_(std::move(guard)), initial_(begin) { }
    public:
        bool DfaCompatible() const noexcept
        {
#pragma warning()
            return true;
        }

        const NfaState* IntialState() const noexcept
        {
            return initial_;
        }

    private:
        Arena arena_;
        NfaState *initial_;
    };

    class NfaBuilder
    {
        static constexpr size_t kDefaultTransitionBufferSize = 2;

    public:
        NfaState* NewState()
        {
            NfaState *result = arena_.Construct<NfaState>();
            result->is_final = false;
            result->exits.ShiftTo(arena_.Allocate<const NfaTransition*>(kDefaultTransitionBufferSize));

            return result;
        }

        NfaTransition* NewEpsilonTransition(NfaState *source, NfaState *target, EpsilonPriority priority)
        {
            NfaTransition *transition = ConstructTransition(source, target, TransitionType::Epsilon);
            transition->data.priority = priority;

            return transition;
        }
        NfaTransition* NewEntityTransition(NfaState *source, NfaState *target, SymbolRange value)
        {
            NfaTransition *transition = ConstructTransition(source, target, TransitionType::Entity);
            transition->data.values = value;

            return transition;
        }
        NfaTransition* NewAnchorTransition(NfaState *source, NfaState *target, AnchorType anchor)
        {
            NfaTransition *transition = ConstructTransition(source, target, TransitionType::Anchor);
            transition->data.anchor = anchor;

            return transition;
        }
        NfaTransition* NewCaptureTransition(NfaState *source, NfaState *target, capture_t id)
        {
            NfaTransition *transition = ConstructTransition(source, target, TransitionType::Capture);
            transition->data.capture_id = id;

            return transition;
        }
        NfaTransition* NewReferenceTransition(NfaState *source, NfaState *target, capture_t id)
        {
            NfaTransition *transition = ConstructTransition(source, target, TransitionType::Reference);
            transition->data.capture_id = id;

            return transition;
        }
        NfaTransition* NewAssertionTransition(NfaState *source, NfaState *target, AssertionType type)
        {
            return ConstructTransition(source, target, TransitionType::Assertion);
        }
        NfaTransition* NewFinishTransition(NfaState *source, NfaState *target)
        {
            return ConstructTransition(source, target, TransitionType::Finish);
        }

        NfaTransition* CloneTransition(NfaState *source, NfaState *target, const NfaTransition *transition)
        {
            NfaTransition *new_transition = ConstructTransition(source, target, transition->type);
            new_transition->data = transition->data;

            return new_transition;
        }
        void CloneBranch(NfaState *source, NfaState *target, NfaBranch branch)
        {
            std::unordered_map<const NfaState*, NfaState*> state_map;
            std::queue<const NfaState*> waitlist;

            state_map.insert_or_assign(branch.begin, source);
            state_map.insert_or_assign(branch.end, target);

            waitlist.push(branch.begin);
            while (!waitlist.empty())
            {
                const NfaState *start = waitlist.front();
                waitlist.pop();

                NfaState *mapped_start = state_map[start];
                for (const NfaTransition *edge : start->exits)
                {
                    NfaState *mapped_target;

                    auto mapped_target_iter = state_map.find(edge->target);
                    if (mapped_target_iter == state_map.end())
                    {
                        mapped_target = NewState();
                        state_map.insert_or_assign(edge->target, mapped_target);
                        waitlist.push(edge->target);
                    }
                    else
                    {
                        mapped_target = mapped_target_iter->second;
                    }

                    CloneTransition(mapped_start, mapped_target, edge);
                }
            }
        }

        NfaAutomaton Build(NfaState *begin)
        {
            return NfaAutomaton{ std::move(arena_), begin };
        }

    private:
        NfaTransition* ConstructTransition(NfaState *source, NfaState *target, TransitionType type)
        {
            // as this is pod type
            // simply allocate it, and construct manually
            NfaTransition *result = arena_.Allocate<NfaTransition>();
            result->source = source;
            result->target = target;
            result->type = type;

            // add the transition into exits in source
            // if buffer in source is full, allocate a larger one
            if (source->exits.Full())
            {
                size_t new_buffer_size = source->exits.Capacity() * 2;
                source->exits.ShiftTo(arena_.Allocate<const NfaTransition*>(new_buffer_size));
            }
            source->exits.PushBack(result);

            return result;
        }

    private:
        Arena arena_;
    };

    // for debug purpose
    void PrintNfa(const NfaAutomaton &atm);
    NfaAutomaton EliminateEpsilon(const NfaAutomaton &atm, bool rich_functional);
    DfaAutomaton GenerateDfa(const NfaAutomaton &atm, SymbolDictionary dict);

} // namespace regex
} // namespace eds