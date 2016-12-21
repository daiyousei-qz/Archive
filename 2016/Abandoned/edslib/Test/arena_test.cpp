#include "catch.hpp"
#include "../Source/arena.hpp"

TEST_CASE("arena::")
{
    eds::Arena arena;
    int *p1 = arena.Construct<int>(42);
    REQUIRE(*p1 == 42);
    for (int i = 0; i < 1000; ++i)
    {
        arena.Allocate(rand() % 3000);
    }
}