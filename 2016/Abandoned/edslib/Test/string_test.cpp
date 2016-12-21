#include "catch.hpp"
#include "../Source/string.hpp"

using namespace eds;

TEST_CASE("ZString::Exhausted")
{
    zstring s1 = "";
    REQUIRE(s1.Exhausted());

    char buf[2] = {};
    // use int to avoid overflow
    for (int ch = 1; ch < 128; ++ch)
    {
        buf[0] = static_cast<char>(ch);
        zstring s2 = zstring{ buf };

        REQUIRE_FALSE(s2.Exhausted());
    }
}

TEST_CASE("ZString::Consume")
{
    zstring s = "hi";

    // first char
    REQUIRE(s.Consume() == 'h');
    // second char
    REQUIRE_FALSE(s.ConsumeIf('h'));
    REQUIRE(s.ConsumeIf('i'));
    // exhausted
    REQUIRE(s.Exhausted());
}

TEST_CASE("ZString::Peek")
{
    zstring s = "hello";

    REQUIRE(s.Peek() == 'h');
    REQUIRE(s.TestPrefix('h'));
    REQUIRE(s.TestPrefix("hell"));
    REQUIRE_FALSE(s.TestPrefix('x'));
    REQUIRE_FALSE(s.TestPrefix("bell"));
}

TEST_CASE("ZString::Length")
{
    REQUIRE(zstring{ "" }.GetLength() == 0);
    REQUIRE(zstring{ "moha" }.GetLength() == 4);
}