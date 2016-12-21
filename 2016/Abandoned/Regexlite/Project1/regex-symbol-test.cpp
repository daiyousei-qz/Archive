#include "catch.hpp"
#include "regex-symbol.h"

using namespace eds;
using namespace eds::regex;

// helpers
namespace
{
    bool IsEqual(const std::vector<SymbolRange> &lhs, const std::vector<SymbolRange> &rhs)
    {
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
            [](SymbolRange r1, SymbolRange r2)
        {
            return r1.min == r2.min && r1.max == r2.max;
        });
    }

    void TestCharSetExpansion(const CharSet &cs, const std::vector<SymbolRange> expected)
    {
        std::vector<SymbolRange> expanded_cs;
        cs.ForEach(
            [&](SymbolRange range)
        {
            expanded_cs.push_back(range);
        });

        REQUIRE(IsEqual(expanded_cs, expected));
    }

    void TestAccumulated(SymbolRange range1, SymbolRange range2,
        const std::vector<SymbolRange> &expected_merged,
        const std::vector<SymbolRange> &expected_disjoint)
    {
        RangeAccumulator accumulator;

        accumulator.Insert(range1);
        accumulator.Insert(range2);

        REQUIRE(IsEqual(accumulator.ExtractMerged(), expected_merged));
        REQUIRE(IsEqual(accumulator.ExtractDisjoint(), expected_disjoint));
    }
}

TEST_CASE("eds::regex::CharSet")
{
    {
        SymbolRange range = { 0x30, 0x7f };
        CharSet cs1{ false, range };
        CharSet cs2{ true, range };

        REQUIRE(cs1.Min() == 0x30);
        REQUIRE(cs1.Max() == 0x7f);
        REQUIRE(cs2.Min() == 0x00);
        REQUIRE(cs2.Max() == 0x10ffff);

        TestCharSetExpansion(cs1, { { 0x30, 0x7f } });
        TestCharSetExpansion(cs2, { { 0x00, 0x30 },{ 0x7f, 0x10ffff } });
    }

    {
        std::vector<SymbolRange> ranges = { { 0x30,0x7f },{ 0xff,0x1000 } };
        CharSet cs1{ false, ranges };
        CharSet cs2{ true, ranges };

        REQUIRE(cs1.Min() == 0x30);
        REQUIRE(cs1.Max() == 0x1000);
        REQUIRE(cs2.Min() == 0x00);
        REQUIRE(cs2.Max() == 0x10ffff);

        TestCharSetExpansion(cs1, { { 0x30,0x7f },{ 0xff,0x1000 } });
        TestCharSetExpansion(cs2, { { 0x00,0x30 },{ 0x7f,0xff },{ 0x1000,0x10ffff } });
    }
}

TEST_CASE("eds::regex::SymbolDictionary")
{
    SymbolDictionary dict = SymbolDictionary{ { { 1,4 },{ 4,8 },{ 9,10 } } };
    REQUIRE(dict.LexemeCount() == 3);
    REQUIRE(dict.Translate(2) == 0);
    REQUIRE(dict.Translate(4) == 1);
    REQUIRE(dict.Translate(9) == 2);

    auto remaped_range = dict.Remap(SymbolRange{ 1,8 });
    REQUIRE((remaped_range.min == 0 && remaped_range.max == 2));
}

TEST_CASE("eds::regex::RangeAccumulator")
{
    //
    // coverage test
    //

    // [...)
    // [...)
    TestAccumulated({ 0,3 }, { 0,3 }, { { 0,3 } }, { { 0,3 } });

    //   [...)
    // [...)
    TestAccumulated({ 2,5 }, { 0,3 }, { { 0,5 } }, { { 0,2 },{ 2,3 },{ 3,5 } });

    //     [...)
    // [...)
    TestAccumulated({ 3,5 }, { 0,3 }, { { 0,5 } }, { { 0,3 },{ 3,5 } });

    // [...)
    // [.....)
    TestAccumulated({ 0,3 }, { 0,5 }, { { 0,5 } }, { { 0,3 },{ 3,5 } });

    //   [...)
    // [.....)
    TestAccumulated({ 2,5 }, { 0,5 }, { { 0,5 } }, { { 0,2 },{ 2,5 } });

    //     [...)
    // [.....)
    TestAccumulated({ 4,7 }, { 0,5 }, { { 0,7 } }, { { 0,4 },{ 4,5 },{ 5,7 } });

    //       [...)
    // [.....)
    TestAccumulated({ 5,8 }, { 0,5 }, { { 0,8 } }, { { 0,5 },{ 5,8 } });

    //
    // misc test
    //
    {
        // merge test
        RangeAccumulator accumulator;
        accumulator.Insert({ 0,7 });
        accumulator.Insert({ 0,4 });
        accumulator.Insert({ 3,5 });
        accumulator.Insert({ 6,9 });
        accumulator.Insert({ 13,19 });
        accumulator.Insert({ 13,16 });

        std::vector<SymbolRange> expected = { { 0,9 },{ 13,19 } };
        REQUIRE(IsEqual(accumulator.ExtractMerged(), expected));
    }

    {
        // disjoin test
        RangeAccumulator accumulator;
        accumulator.Insert({ 0,26 });
        accumulator.Insert({ 5,8 });
        accumulator.Insert({ 5,13 });
        accumulator.Insert({ 16,19 });
        accumulator.Insert({ 18,20 });
        accumulator.Insert({ 23,24 });

        std::vector<SymbolRange> expected = 
            { { 0,5 },{ 5,8 },{ 8,13 },{ 13,16 },{ 16,18 }
              ,{ 18,19 },{ 19,20 },{ 20,23 },{ 23,24 },{ 24,26 } };

        REQUIRE(IsEqual(accumulator.ExtractDisjoint(), expected));
    }
}