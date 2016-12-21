#include "catch.hpp"
#include "../Source/lhelper.hpp"
#include "../Source/compact_set.hpp"
#include <array>
#include <algorithm>

using namespace eds;

TEST_CASE("CompactSet::ctor")
{
    std::array<int, 4> arr = { 1,2,3,4 };
    // via initializaer_list
    compact_set<int> s1 = { 1,2,3,4 };
    // via iterator
    compact_set<int> s2{ arr.begin(), arr.end() };
    // via copy ctor
    compact_set<int> s3{ s1 };
    // via move ctor
    compact_set<int> s4_{ s1 };
    compact_set<int> s4{ std::move(s4_) };

    auto test_set = [&arr](compact_set<int>& s)
    {
        Asserts(s.size() == arr.size());
        // Note set is ordered
        Asserts(std::equal(s.begin(), s.end(), arr.begin(), arr.end()));
    };
    test_set(s1);
    test_set(s2);
    test_set(s3);
    test_set(s4);
}

TEST_CASE("CompactSet::access")
{
	compact_set<int> s = { 1,2,3,4,5,6,7,8 };
	Asserts(s.count(1) == 1);
	Asserts(s.count(0) == 0);
	Asserts(s.find(9) == s.end());
	Asserts(s.at(3) == 4);
	Asserts(s[6] == 7);
}

TEST_CASE("CompactSet::swap")
{
	compact_set<int> v1 = { 1,2,41,16,7 };
	compact_set<int> v2 = { 1,-1,6,2,77,21,2,5,7 };

	auto v3 = v1;
	auto v4 = v2;
	v3.swap(v4);
	Asserts(v3 == v2 && v4 == v1);
	v3.swap(v4);
	Asserts(v3 == v1 && v4 == v2);
}