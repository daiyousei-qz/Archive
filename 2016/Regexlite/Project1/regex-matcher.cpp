#include "regex-def.h"
#include "regex-text.h"
#include "regex-matcher.h"
#include "regex-parser.h"
#include "regex-automaton.h"
#include "regex-algorithm.h"

namespace
{
    using namespace eds;
    using namespace eds::regex;

    inline RegexMatch CreateFailedMatch()
    {
        return RegexMatch{ false, "", {} };
    }

    inline RegexMatch CreateSucceededMatch(StringView ref)
    {
        return RegexMatch{ true, ref, {} };
    }
}

namespace eds {
namespace regex {

    // Dfa
    class DfaRegexMatcher : public RegexMatcher
    {
    public:
        DfaRegexMatcher(RegexOption option, DfaAutomaton atm)
            : RegexMatcher(option), atm_(std::move(atm)) { }

    protected:
        RegexMatch MatchPrefix(StringView view) const override
        {
            Utf8Reader reader{ view };

            int state = atm_.IntialState();
            int last_accepting_pos = -1;
            while (!reader.Exhausted())
            {
                state = atm_.Transit(state, reader.Read());
                if (state != DfaAutomaton::InvalidState())
                {
                    // record the current position if it's accepting
                    if (atm_.IsAccepting(state))
                    {
                        last_accepting_pos = reader.Cursor();
                    }
                }
                else
                {
                    if (last_accepting_pos != -1)
                    {
                        return CreateSucceededMatch(view.SubString(0, last_accepting_pos));
                    }
                    else
                    {
                        break;
                    }
                }
            }

            return CreateFailedMatch();
        }
        RegexMatch MatchSubString(StringView view) const override
        {
            Utf8Reader reader{ view };

            bool found = false;
            int matched_begin = DfaAutomaton::InvalidState();
            int matched_end = DfaAutomaton::InvalidState();
            int state = atm_.IntialState();
            while (!reader.Exhausted())
            {
                // find matched_begin
                while (!reader.Exhausted())
                {
                    size_t cursor = reader.Cursor();
                    char32_t codepoint = reader.Read();
                    int target_state = atm_.Transit(state, codepoint);
                    if (target_state != DfaAutomaton::InvalidState())
                    {
                        if (atm_.IsAccepting(target_state))
                        {
                            matched_end = reader.Cursor();
                            found = true;
                        }

                        state = target_state;
                        matched_begin = cursor;
                        break;
                    }
                }

                // if matched_begin is found, find matched_end
                while (!reader.Exhausted())
                {
                    char32_t codepoint = reader.Read();
                    state = atm_.Transit(state, codepoint);
                    if (state != DfaAutomaton::InvalidState())
                    {
                        // record the current position if it's accepting
                        if (atm_.IsAccepting(state))
                        {
                            matched_end = reader.Cursor();
                        }
                    }
                    else // state = -1, no more character wanted
                    {
                        if (matched_end != DfaAutomaton::InvalidState())
                        {
                            found = true;
                        }

                        break;
                    }
                }

                if (found)
                {
                    return CreateSucceededMatch(view.SubString(matched_begin, matched_end - matched_begin));
                }
                else if (matched_begin != DfaAutomaton::InvalidState())
                {
                    // if any chance to fallback
                    reader.Seek(matched_begin);
                    reader.Read();
                    state = atm_.IntialState();
                    matched_begin = matched_end = DfaAutomaton::InvalidState();
                }
            }

            return CreateFailedMatch();
        }
    private:
        DfaAutomaton atm_;
    };

    // Thompson's way to evaluate Nfa
    class NfaRegexMatcher : public RegexMatcher
    {
    public:
        NfaRegexMatcher(RegexOption option, NfaAutomaton atm)
            : RegexMatcher(option), atm_(std::move(atm)) { }

    protected:
        RegexMatch MatchPrefix(StringView view) const override
        {
            throw 0;
        }
        RegexMatch MatchSubString(StringView view) const override
        {
            throw 0;
        }

    private:
        NfaAutomaton atm_;
    };

    // backtrack matcher
    class RichNfaRegexMatcher : public RegexMatcher
    {
    public:
        RichNfaRegexMatcher(RegexOption option, NfaAutomaton atm)
            : RegexMatcher(option), atm_(std::move(atm)) { }

    protected:
        RegexMatch MatchPrefix(StringView view) const override
        {
            throw 0;
        }
        RegexMatch MatchSubString(StringView view) const override
        {
            throw 0;
        }

    private:
        NfaAutomaton atm_;
    };

    RegexMatcher::Ptr CreateMatcher(StringView regex, RegexOption option)
    {
        Asserts(option.right_to_left == false);

        // parse
        RegexExpr::Ptr expr = ParseRegex(regex, option);
        if (option.matcher == MatcherType::Dfa)
        {
            auto dict = RewriteSymbolsInvoker(expr->Root());
            auto epsilon_nfa = CreateEpsilonNfaInvoker(expr->Root(), option.right_to_left);
            auto dfa = GenerateDfa(epsilon_nfa, std::move(dict));

            return std::make_unique<DfaRegexMatcher>(option, std::move(dfa));
        }
        else if (option.matcher == MatcherType::Nfa)
        {
            auto epsilon_nfa = CreateEpsilonNfaInvoker(expr->Root(), option.right_to_left);
            auto nfa = EliminateEpsilon(epsilon_nfa, true);

            return std::make_unique<NfaRegexMatcher>(option, std::move(nfa));
        }
        else if (option.matcher == MatcherType::RichNfa)
        {
            auto epsilon_nfa = CreateEpsilonNfaInvoker(expr->Root(), option.right_to_left);
            auto nfa = EliminateEpsilon(epsilon_nfa, false);

            return std::make_unique<RichNfaRegexMatcher>(option, std::move(nfa));
        }

        Asserts(false);
    }
    
} // namespace regex
} // namespace eds
