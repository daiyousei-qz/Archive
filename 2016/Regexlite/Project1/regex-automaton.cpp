#include "regex-model.h"
#include "regex-automaton.h"
#include "compact_set.hpp"
#include <functional>
#include <stack>
#include <queue>
#include <deque>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace
{
    using namespace eds::regex;

    inline bool TransitionMorePrior(const NfaTransition *lhs, const NfaTransition *rhs)
    {
        static const auto CalcTransitionPriority = 
            [](const NfaTransition *edge)
        {
            if (edge->type == TransitionType::Epsilon)
            {
                switch (edge->data.priority)
                {
                case EpsilonPriority::High:
                    return 0;
                case EpsilonPriority::Normal:
                    return 1;
                case EpsilonPriority::Low:
                    return 2;
                default:
                    Asserts(false);
                }
            }
            else
            {
                return 1;
            }
        };

        return CalcTransitionPriority(lhs) < CalcTransitionPriority(rhs);
    }

    inline void EnumerateNfa(const NfaState *initial, std::function<void(const NfaState *)> callback)
    {
        std::unordered_set<const NfaState*> visited;
        std::queue<const NfaState*> waitlist;
        
        waitlist.push(initial);
        visited.insert(initial);
        while (!waitlist.empty())
        {
            const NfaState *source = waitlist.front();
            waitlist.pop();

            callback(source);
            
            for (const NfaTransition *edge : source->exits)
            {
                if (visited.find(edge->target) == visited.end())
                {
                    waitlist.push(edge->target);
                    visited.insert(edge->target);
                }
            }
        }
    }

    struct NfaEvaluatedResult
    {
        const NfaState *initial;
        std::unordered_set<const NfaState*> solid_states;
        std::unordered_set<const NfaState*> accepting_states;
        std::unordered_multimap<const NfaState*, const NfaTransition*> outbounds;
    };

    inline NfaEvaluatedResult EvaluateNfa(const NfaAutomaton &atm)
    {
        // solid states are thoes who has an incoming non-epsilon transition
        // NOTE that only solid states would be calculated
        NfaEvaluatedResult result;

        std::queue<const NfaState*> waitlist;             // wait queue for unprocessed solid states

        // initialize
        const NfaState *initial_state = atm.IntialState();
        result.initial = initial_state;
        result.solid_states.insert(initial_state);
        waitlist.push(initial_state);

        // process until no solid states can be accessed
        while (!waitlist.empty())
        {
            // fetch a source solid state to process
            const NfaState *source = waitlist.front();
            waitlist.pop();

            std::unordered_set<const NfaTransition*> expanded; // to track expanded epsilon transitions
            std::vector<const NfaTransition*> output_buffer;   // to store results of expansion
            std::vector<const NfaTransition*> input_buffer;    // to store transitions to be expanded

            static const auto ExpandTransitions =
                [](std::vector<const NfaTransition*> &output, const NfaState *state)
            {
                auto range_begin_iter = output.insert(output.end(), state->exits.begin(), state->exits.end());
                auto range_end_iter = output.end();

                // NOTE more prior transitions are in front of those less prior ones
                std::sort(range_begin_iter, range_end_iter, TransitionMorePrior);
            };

            // the state which is final is accepting
            if (source->is_final)
            {
                result.accepting_states.insert(source);
            }

            // make initial expansion from source state
            ExpandTransitions(output_buffer, source);
            // iterate to expand all epsilon transitions
            bool has_expansion = true;
            while (has_expansion)
            {
                has_expansion = false;
                input_buffer.clear();
                std::swap(input_buffer, output_buffer);

                for (const NfaTransition* edge : input_buffer)
                {
                    if (edge->type == TransitionType::Epsilon)
                    {
                        if (edge->target->is_final)
                        {
                            // the state which can reach the final state with epsilon only is accepting
                            result.accepting_states.insert(source);
                        }

                        // expand the epsilon transition for the first time only
                        if (expanded.find(edge) == expanded.end())
                        {
                            has_expansion = true;
                            expanded.insert(edge);

                            ExpandTransitions(output_buffer, edge->target);
                        }
                    }
                    else
                    {
                        // if a new solid state, insert it into waitlist
                        if (result.solid_states.find(edge->target) == result.solid_states.end())
                        {
                            result.solid_states.insert(edge->target);
                            waitlist.push(edge->target);
                        }

                        // non-epsilon transtion is simply copyed
                        output_buffer.push_back(edge);
                    }
                }
            }

#pragma warning()
            // eliminate identical transitions(I don't konw if this always works)
            auto new_end_iter = std::unique(output_buffer.begin(), output_buffer.end());
            output_buffer.erase(new_end_iter, output_buffer.end());
            // copy posible transitions into result
            for (const NfaTransition *edge : output_buffer)
            {
                result.outbounds.insert({ source, edge });
            }
        }

        return result;
    }
}

namespace eds {
namespace regex {

    void PrintNfa(const NfaAutomaton &atm)
    {
        int next_id = 0;
        std::unordered_map<const NfaState*, int> id_map;
        id_map.insert_or_assign(atm.IntialState(), next_id++);

        EnumerateNfa(atm.IntialState(),
            [&](const NfaState *source)
        {
            int source_id = id_map[source];

            // print title
            printf("NfaState %d", source_id);
            if (source->is_checkpoint)
            {
                printf("[checkpoint]");
            }
            if (source->is_final)
            {
                printf("(final)");
            }

            printf(":\n");

            // foreach outgoing edges
            for (const NfaTransition *edge : source->exits)
            {
                printf("  ");

                // print type of the edge
                switch (edge->type)
                {
                case TransitionType::Epsilon:
                {
                    const char *priority = "";
                    switch (edge->data.priority)
                    {
                    case EpsilonPriority::High:
                        priority = "high";
                        break;
                    case EpsilonPriority::Normal:
                        priority = "normal";
                        break;
                    case EpsilonPriority::Low:
                        priority = "low";
                        break;
                    }

                    printf("Epsilon(%s)", priority);
                }
                    break;
                case TransitionType::Entity:
                    printf("Codepoint(%c, %c)", 
                        edge->data.values.min, 
                        edge->data.values.max);
                    break;
                case TransitionType::Anchor:
                    printf("Anchor(%s)", edge->data.anchor == AnchorType::Dollar ? "$" : "^");
                    break;
                case TransitionType::Capture:
                    printf("Capture(%d)", edge->data.capture_id);
                    break;
                case TransitionType::Reference:
                    printf("Reference(%d)", edge->data.capture_id);
                    break;
                case TransitionType::Assertion:
                    printf("Assertion");
                {
                    switch (edge->data.assertion)
                    {
                    case AssertionType::PositiveLookAhead:
                        printf("(PositiveLookAhead)");
                        break;
                    case AssertionType::NegativeLookAhead:
                        printf("(NegativeLookAhead)");
                        break;
                    case AssertionType::PositiveLookBehind:
                        printf("(PositiveLookBehid)");
                        break;
                    case AssertionType::NegativeLookBehind:
                        printf("(NegativeLookBehind)");
                        break;
                    default:
                        break;
                    }
                    printf("\n");
                }
                    break;
                case TransitionType::Finish:
                    printf("(finish)");
                    break;
                default:
                    Asserts(false);
                }

                // print target id, generate one if needed
                int target_id;
                auto target_iter = id_map.find(edge->target);
                if (target_iter == id_map.end())
                {
                    target_id = next_id++;
                    id_map.insert_or_assign(edge->target, target_id);
                }
                else
                {
                    target_id = target_iter->second;
                }

                printf("  => NfaState %d\n", target_id);

            }
        });
    }
    
    NfaAutomaton EliminateEpsilon(const NfaAutomaton &atm, bool /*not used*/ core_only)
    {
        NfaEvaluatedResult eval = EvaluateNfa(atm);
        NfaBuilder builder;
        std::unordered_map<const NfaState*, NfaState*> state_map;

        // first iteration: clone states
        for (const NfaState *state : eval.solid_states)
        {
            NfaState *mapped_state = builder.NewState();
            mapped_state->is_final = eval.accepting_states.find(state) != eval.accepting_states.end();

            state_map.insert_or_assign(state, mapped_state);
        }

        // second iteration: clone transitions
        for (const NfaState *source : eval.solid_states)
        {
            Asserts(state_map.find(source) != state_map.end());

            std::vector<SymbolRange> passing_lexemes;

            bool checkpoint_test = false;       // indicator of possible ambiguity
            size_t counter = 0;                 // count of transitions
            auto mapped_source = state_map[source];
            auto outgoing_edges = eval.outbounds.equal_range(source);

            // clone transitions one by one
            std::for_each(outgoing_edges.first, outgoing_edges.second,
                [&](auto pair)
            {
                // fetch edge
                const NfaTransition *edge = pair.second;

                // increment counter of total transitions
                counter += 1;

                // clone transition
                Asserts(edge->type != TransitionType::Epsilon);
                Asserts(state_map.find(edge->target) != state_map.end());
                builder.CloneTransition(mapped_source, state_map[edge->target], edge);

                // test if checkpoint should be set
                if (!checkpoint_test)
                {
                    if (edge->type != TransitionType::Entity)
                    {
                        checkpoint_test = true;
                    }
                    else
                    {
                        // to test if any of passing lexemes overlaps with condition
                        // i.e. there's ambiguity
                        SymbolRange condition = edge->data.values;
                        checkpoint_test = std::any_of(passing_lexemes.begin(), passing_lexemes.end(),
                            [condition](SymbolRange range)
                        {
                            return std::max(condition.min, range.min) < std::min(condition.max, range.max);
                        });

                        if (!checkpoint_test)
                        {
                            passing_lexemes.push_back(condition);
                        }
                    }
                }
            });

            mapped_source->is_checkpoint = counter > 1 && checkpoint_test;
        }

        Asserts(state_map.find(eval.initial) != state_map.end());
        return builder.Build(state_map[eval.initial]);
    }

    DfaAutomaton GenerateDfa(const NfaAutomaton &atm, SymbolDictionary dict)
    {
        NfaEvaluatedResult eval = EvaluateNfa(atm);
        size_t symbol_cnt = dict.LexemeCount();
        DfaBuilder builder{ std::move(dict) };

        using NfaStateSet = compact_set<const NfaState*>;
        using DfaStateId = decltype(builder.NewState(false));
        std::map<NfaStateSet, DfaStateId> id_map;
        std::queue<NfaStateSet> waitlist;

        const auto TestAccepting =
            [&](const NfaState *state)
        {
            return eval.accepting_states.find(state) != eval.accepting_states.end();
        };
        
        // process initial state
        // initial state cannot be accepting as regex should not be nullable
        Asserts(!TestAccepting(eval.initial));
        DfaStateId initial_id = builder.NewState(false);
        NfaStateSet initial_set = NfaStateSet{ eval.initial };
        id_map.insert_or_assign(initial_set, initial_id);

        waitlist.push(initial_set);

        while (!waitlist.empty())
        {
            // fetch source set(move)
            NfaStateSet source_set = std::move(waitlist.front());
            DfaStateId source_id = id_map[source_set];
            // discard queued item
            waitlist.pop();

            // make a copy of all outgoing transitions
            std::vector<const NfaTransition*> transitions;
            for (const NfaState* state : source_set)
            {
                auto range = eval.outbounds.equal_range(state);
                std::transform(range.first, range.second, std::back_inserter(transitions), 
                    [](auto iter) { return iter.second; });
#pragma warning()
            }

            Asserts(std::all_of(transitions.begin(), transitions.end(),
                [](const NfaTransition *edge) {return edge->type == TransitionType::Entity; }));

            // for each possible symbol
            for (size_t s = 0; s < symbol_cnt; ++s)
            {
                // calculate target dfa set
                NfaStateSet target_set;
                for (const NfaTransition *edge : transitions)
                {
                    if (edge->data.values.Contain(s))
                    {
                        target_set.insert(edge->target);
                    }
                }

                // empty target_set is invalid, thus not considered
                if (!target_set.empty())
                {
                    // calculate dfa id for target_set
                    DfaStateId target_id;
                    auto id_iter = id_map.find(target_set);
                    if (id_iter != id_map.end())
                    {
                        target_id = id_iter->second;
                    }
                    else
                    {
                        // state not found in cache
                        // so create a new one
                        bool accepting = std::any_of(target_set.begin(), target_set.end(), TestAccepting);
                        target_id = builder.NewState(accepting);
                        id_map.insert_or_assign(target_set, target_id);

                        // queue it
                        waitlist.push(std::move(target_set));
                    }

                    // make transition
                    builder.NewTransition(source_id, target_id, s);
                }
            }
        }

        return builder.Build();
    }

}
}
