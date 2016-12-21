#include "catch.hpp"
#include "regex-parser.h"

using namespace eds;
using namespace eds::regex;

void DefaultParse(StringView regex)
{
    RegexOption option = RegexOption::RichNfaDefault();
    ParseRegex(regex, option);
}

TEST_CASE("good construction", "[parser]")
{
    // concatenation
    REQUIRE_NOTHROW(DefaultParse("ab"));
    // alternation
    REQUIRE_NOTHROW(DefaultParse("a|b"));
    // repetition
    REQUIRE_NOTHROW(DefaultParse("a?"));
    REQUIRE_NOTHROW(DefaultParse("a*"));
    REQUIRE_NOTHROW(DefaultParse("a+"));
    REQUIRE_NOTHROW(DefaultParse("a{1}"));
    REQUIRE_NOTHROW(DefaultParse("a{1,}"));
    REQUIRE_NOTHROW(DefaultParse("a{1,2}"));
    REQUIRE_NOTHROW(DefaultParse("a??"));
    REQUIRE_NOTHROW(DefaultParse("a*?"));
    REQUIRE_NOTHROW(DefaultParse("a+?"));
    REQUIRE_NOTHROW(DefaultParse("a{3,}?"));
    // grouping
    REQUIRE_NOTHROW(DefaultParse("(ab)x"));
    REQUIRE_NOTHROW(DefaultParse("(?:ab)x"));
    REQUIRE_NOTHROW(DefaultParse("(?<name>ab)x"));
    REQUIRE_NOTHROW(DefaultParse("(?=ab)x"));
    REQUIRE_NOTHROW(DefaultParse("(?!ab)x"));
    REQUIRE_NOTHROW(DefaultParse("(?<=ab)x"));
    REQUIRE_NOTHROW(DefaultParse("(?<!ab)x"));
    // escaped character
    REQUIRE_NOTHROW(DefaultParse("\\a"));
    REQUIRE_NOTHROW(DefaultParse("\\r"));
    REQUIRE_NOTHROW(DefaultParse("\\n"));
#pragma warning() // such test is rediculous though
    REQUIRE_NOTHROW(DefaultParse("\\040"));
    REQUIRE_NOTHROW(DefaultParse("\\x30"));
    REQUIRE_NOTHROW(DefaultParse("\\u0030"));

    // charactor class
    REQUIRE_NOTHROW(DefaultParse("[abc]"));
    REQUIRE_NOTHROW(DefaultParse("[a-z9-0]"));
    REQUIRE_NOTHROW(DefaultParse("[^a-z_]"));
    REQUIRE_NOTHROW(DefaultParse("[-+*/]"));
    // escaped character class
    REQUIRE_NOTHROW(DefaultParse("\\w"));
    REQUIRE_NOTHROW(DefaultParse("\\W"));
    REQUIRE_NOTHROW(DefaultParse("\\d"));
    REQUIRE_NOTHROW(DefaultParse("\\D"));
    REQUIRE_NOTHROW(DefaultParse("\\s"));
    REQUIRE_NOTHROW(DefaultParse("\\S"));
    // escaped metachar
    REQUIRE_NOTHROW(DefaultParse("\\(\\)\\[\\]\\{\\}\\?\\*\\+\\\\"));
}

TEST_CASE("bad construction", "[parser]")
{
    ///* empty regex *///
    // empty regex
    REQUIRE_THROWS_AS(DefaultParse(""), RegexConstructionError);
    // empty regex with alternation
    REQUIRE_THROWS_AS(DefaultParse("|"), RegexConstructionError);
    // empty regex with group
    REQUIRE_THROWS_AS(DefaultParse("()"), RegexConstructionError);
    // unbalanced parenthesis
    REQUIRE_THROWS_AS(DefaultParse("("), RegexConstructionError);
    REQUIRE_THROWS_AS(DefaultParse(")"), RegexConstructionError);
    REQUIRE_THROWS_AS(DefaultParse("(a"), RegexConstructionError);

    ///* char class *///
    // empty char class
    REQUIRE_THROWS_AS(DefaultParse("[]"), RegexConstructionError);
    REQUIRE_THROWS_AS(DefaultParse("[^]"), RegexConstructionError);
    // incomplete or invalid char class
    REQUIRE_THROWS_AS(DefaultParse("["), RegexConstructionError);
    REQUIRE_THROWS_AS(DefaultParse("[a"), RegexConstructionError);

    ///* closure *///
    // isolated ? closure
    REQUIRE_THROWS_AS(DefaultParse("?"), RegexConstructionError);
    // isolated + closure
    REQUIRE_THROWS_AS(DefaultParse("+"), RegexConstructionError);
    // isolated * closure
    REQUIRE_THROWS_AS(DefaultParse("*"), RegexConstructionError);
    // isolated explicit closure
    REQUIRE_THROWS_AS(DefaultParse("{1}"), RegexConstructionError);
    // empty explicit closure
    REQUIRE_THROWS_AS(DefaultParse("a{}"), RegexConstructionError);
    REQUIRE_THROWS_AS(DefaultParse("a{,}"), RegexConstructionError);
    // implicit least repitition
    REQUIRE_THROWS_AS(DefaultParse("a{,1}"), RegexConstructionError);
    // uncolsed brace
    REQUIRE_THROWS_AS(DefaultParse("a{1"), RegexConstructionError);
}