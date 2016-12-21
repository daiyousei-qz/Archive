#pragma once
#include "lhelper.hpp"
#include <stdexcept>
#include <bitset>

namespace eds {
namespace regex {
    
    enum class MatcherType
    {
        Dfa, Nfa, RichNfa
    };

    class RegexOption2
    {
    public:
        enum
        {
            kIgnoreCase,
            kIgnoreWhitespace,
            kImplicitCapture,
            kMultilineMode,
            kRightToLeft
        };

        using OptionType = decltype(kIgnoreCase);

    public:
        void Set(OptionType opt, bool value)
        {
            flags_.set(opt, value);
        }

        bool Test(OptionType kname)
        {
            return flags_.test(kname);
        }

    private:
        std::bitset<32> flags_;
    };

    struct RegexOption
    {
        MatcherType matcher     = MatcherType::Nfa;
        bool ignore_case        = false;
        bool ignore_whitespace  = false;
        bool implicit_capture   = false;
        bool multiline_mode     = false;
        bool right_to_left      = false;

        bool Validate() const noexcept
        {
            bool test = true;

            if (implicit_capture)
            {
                test &= matcher == MatcherType::RichNfa;
            }

            return test;
        }

    public:
        static RegexOption DfaDefault()
        {
            RegexOption option;
            option.matcher = MatcherType::Dfa;
            option.ignore_case = false;
            option.ignore_whitespace = false;
            option.implicit_capture = false;
            option.multiline_mode = true;
            option.right_to_left = false;

            return option;
        }

        static RegexOption NfaDefault()
        {
            RegexOption option;
            option.matcher = MatcherType::Nfa;
            option.ignore_case = false;
            option.ignore_whitespace = false;
            option.implicit_capture = false;
            option.multiline_mode = true;
            option.right_to_left = false;

            return option;
        }

        static RegexOption RichNfaDefault()
        {
            RegexOption option;
            option.matcher = MatcherType::RichNfa;
            option.ignore_case = false;
            option.ignore_whitespace = false;
            option.implicit_capture = true;
            option.multiline_mode = true;
            option.right_to_left = false;

            return option;
        }
    };

    //=================================================================================
    // Errors
    class RegexConstructionError : std::runtime_error
    {
    public:
        RegexConstructionError(const char *msg)
            : std::runtime_error(msg) { }
    };

    class RegexEvaluationError : std::runtime_error
    {
    public:
        RegexEvaluationError(const char *msg)
            : std::runtime_error(msg) { }
    };

} // namespace regex
} // namespace eds