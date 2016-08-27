#pragma once
#include "regex-def.h"
#include "string.hpp"
#include <vector>
#include <memory>

/*
grammar for regex

-chacters
[character_group]       ...
[^chacter_group]        ...
[first_char-last_char]  ...
.                       all chacter
\w                      all alphabets
\W                      all non-alphabets
\s                      all whitespace
\S                      all non-whitespace
\d                      all digits
\D                      all non-digit
-anchor
$                       start of string or line
^                       end of string or line

-grouping
(subexpression)         ...
(?<name>subexpression)  named capturing
(?=subexpression)       positive lookahead
(?!subexpression)       negative lookahead

-quantifier
*                       match 0 to inf times
?                       match 0 to 1 times
+                       match 1 to inf times
{n}                     ...
{n,}                    ...
{m,n}                   ...
extra-?                 reluctant closure

-backreference
\k<name>

*/

namespace eds {
namespace regex {

    struct RegexMatch
    {
        bool success;
        StringView content;
        std::vector<StringView> capture;
    };

    using RegexMatches = std::vector<RegexMatch>;

    class RegexMatcher
    {
    public:
        using Ptr = std::unique_ptr<RegexMatcher>;

    public:
        RegexMatcher(RegexOption option)
            : option_(option) { }

    public:
        RegexOption Option() const noexcept
        {
            return option_;
        }

        RegexMatch Search(StringView view)
        {
            return MatchSubString(view);
        }

        RegexMatches SearchAll(StringView view)
        {
            RegexMatches result;
            StringView remaining_view = view;

            while (!remaining_view.IsEmpty())
            {
                RegexMatch match = MatchSubString(remaining_view);
                if (match.success)
                {
                    // truncate remaining view to search next
                    size_t searched_offset = std::distance(view.FrontPointer(), match.content.BackPointer());
                    remaining_view = view.RemovePrefix(searched_offset);

                    // save the last successful match
                    result.push_back(std::move(match));
                }
                else
                {
                    // no more matches
                    break;
                }
            }

            return result;
        }

    protected:
        // NOTE MatchPrefix and MatchSubString are basic operations
        // that is implemented differently by each derived matcher

        // try to match a prefix of str
        virtual RegexMatch MatchPrefix(StringView view) const = 0;
        // try to match a substring of str
        virtual RegexMatch MatchSubString(StringView view) const = 0;

    private:
        RegexOption option_;
    };

    RegexMatcher::Ptr CreateMatcher(StringView expression, RegexOption option);

} // namespace regex
} // namespace eds