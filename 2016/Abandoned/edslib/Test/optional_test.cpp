#include "catch.hpp"
#include "../Source/optional.hpp"

struct TestInt
{
public:
    TestInt() = default;
    TestInt(const TestInt &other)
        : value(other.value) 
    {
    }
    TestInt(TestInt &&other)
        : value(other.value)
    {
        other.value = 0;
    }
    TestInt(int x)
        : value(x)
    {
    }

    ~TestInt()
    {
        value = 0;
    }

    int Negate() const
    {
        return -value;
    }

    bool operator==(const TestInt &rhs) const
    {
        return value == rhs.value;
    }
    bool operator==(int rhs) const
    {
        return value == rhs;
    }

public:
    int value = 0;
};

using optional = eds::optional<TestInt>;

TEST_CASE("optional::ctor_disengaged")
{

    // default constructed
    optional o1{};
    REQUIRE_FALSE(o1);
    // constructed with nullopt
    optional o2{ eds::nullopt };
    REQUIRE_FALSE(o2);
    // copy constructed
    optional o3 = o2;
    REQUIRE_FALSE(o3);

    REQUIRE(o1 == eds::nullopt);
    REQUIRE(o1 == optional{});
    REQUIRE(o1 == o2);
    REQUIRE(o2 == o3);
    REQUIRE(o3 == o1);
}

TEST_CASE("optional::ctor_engaged")
{
    TestInt i1{ 42 }, i2{ 42 };
    // copy constructed from T
    optional o3{ i1 };
    REQUIRE(o3.value() == 42);
    // move constructed from T
    optional o4{ std::move(i2) };
    REQUIRE(*o4 == 42);
    // copy constructed from optional
    optional o5{ o3 };
    REQUIRE(o5.value() == 42);
    // move constructed from optional
    optional o6{ std::move(o4) };
    REQUIRE(o6.value() == 42);
    // in place constructed
    optional o7{ eds::in_place, 42 };
    REQUIRE(o7.value() == 42);
}

TEST_CASE("optional::move_behavior")
{
    TestInt x = 42;
    // move constructed from T
    optional o1 = std::move(x);
    REQUIRE(o1);
    REQUIRE(*o1 == 42);
    REQUIRE(x == 0);
    // move constructed from optional
    optional o2 = std::move(o1);
    REQUIRE(o2);
    REQUIRE(*o2 == 42);
    REQUIRE(o1);
    REQUIRE(*o1 == 0);

    TestInt y = 42;
    // move assigned from T
    optional o3;
    o3 = std::move(y);
    REQUIRE(o3);
    REQUIRE(*o3 == 42);
    REQUIRE(y == 0);
    // move assigned from optional
    optional o4;
    o4 = std::move(o3);
    REQUIRE(o4);
    REQUIRE(*o4 == 42);
    REQUIRE(o3);
    REQUIRE(*o3 == 0);
}
TEST_CASE("optional::has_value")
{
    // empty one
    optional o1{};
    REQUIRE_FALSE(o1);
    REQUIRE_FALSE(o1.has_value());
    REQUIRE_THROWS_AS(o1.value(), eds::bad_optional_access);

    // engaged one
    optional o2{ 42 };
    REQUIRE(o2);
    REQUIRE(o2.has_value());
    REQUIRE(o2.value() == 42);
}

TEST_CASE("optional::access")
{
    for (int i = 40; i <= 42; ++i)
    {
        optional o1{ i };
        // direct access
        REQUIRE(o1.value() == i);
        REQUIRE(*o1 == i);
        // delegated access
        REQUIRE(o1->value == i);
        REQUIRE(o1->Negate() == -i);
    }

    optional o2{ 42 };
    REQUIRE(o2.value_or(41) == 42);
    REQUIRE(std::move(o2).value_or(41) == 42);

    optional o3{};
    REQUIRE(o3.value_or(41) == 41);
    REQUIRE(std::move(o3).value_or(41) == 41);
}

TEST_CASE("optional::reset")
{
    optional o1{ 42 };
    o1.reset();
    CHECK(*o1 == 0); // trick to ensure the instance is cleared
    REQUIRE_FALSE(o1);
}