#include "catch.hpp"
#include "../Source/static_vector.hpp"
#include <tuple>
#include <array>
#include <algorithm>

using namespace eds;

TEST_CASE("StaticVector::ctor")
{
	std::array<int, 4> arr = { 1,2,3,4 };
	// via initializaer_list
	static_vector<int> v1 = { 1,2,3,4 };
	// via iterator
    static_vector<int> v2{ arr.begin(), arr.end() };
	// via copy ctor
    static_vector<int> v3{ v1 };
	// via move ctor
    static_vector<int> v4_{ v1 };
    static_vector<int> v4{ std::move(v4_) };

	auto test_vector = [&arr](static_vector<int>& v)
	{
		REQUIRE(v.size() == arr.size());
        REQUIRE(std::equal(v.begin(), v.end(), arr.begin(), arr.end()));
	};
	test_vector(v1);
	test_vector(v2);
	test_vector(v3);
	test_vector(v4);
}

TEST_CASE("StaticVector::push")
{
	using point = std::tuple<int, int>;
	std::array<point, 3> arr = { point{1,1}, point{2,2}, point{3,3} };
    static_vector<point> v;
	v.emplace_back(1, 1);
	v.push_back(point{ 2,2 });
	point p{ 3,3 };
	v.push_back(p);

    REQUIRE(v.size() == arr.size());
    REQUIRE(std::equal(v.begin(), v.end(), arr.begin(), arr.end()));
}

TEST_CASE("StaticVector::access")
{
    static_vector<int> v = { 1,2,3,4,5,6,7,8 };
    REQUIRE(v.front() == 1);
    REQUIRE(v.back() == 8);
    REQUIRE(v.at(3) == 4);
    REQUIRE(v[6] == 7);
}

TEST_CASE("StaticVector::insert")
{
	std::array<int, 10> arr = { 2,9,9,9,3,6,6,6,1,2 };
    static_vector<int> v = { 1,2 };
	v.insert(v.begin(), { 6,6,6 });
	std::array<int, 2> ct = { 2,3 };
	v.insert(v.begin(), ct.begin(), ct.end());
	v.insert(std::next(v.begin()), 3, 9);

    REQUIRE(v.size() == arr.size());
    REQUIRE(std::equal(v.begin(), v.end(), arr.begin(), arr.end()));
}

TEST_CASE("StaticVector::erase")
{
	std::array<int, 4> arr = { 1,2,5,6 };
    static_vector<int> v = { 1,2,3,4,5,6 };
	v.erase(v.begin() + 2, v.begin() + 4);

    REQUIRE(v.size() == arr.size());
    REQUIRE(std::equal(v.begin(), v.end(), arr.begin(), arr.end()));
}

TEST_CASE("StaticVector::swap")
{
    static_vector<int> v1 = { 1,2,41,16,7 };
    static_vector<int> v2 = { 1,-1,6,2,77,21,2,5,7 };
	
	static_vector<int> v3 = v1;
	static_vector<int> v4 = v2;
	v3.swap(v4);
    REQUIRE(v3 == v2);
    REQUIRE(v4 == v1);
	v3.swap(v4);
    REQUIRE(v3 == v1);
    REQUIRE(v4 == v2);
}

TEST_CASE("StaticVector::compare")
{
    static_vector<int> v1 = { 1,2,3,4 };
    static_vector<int> v2 = { 1,2,3,4 };
    static_vector<int> v3 = { 1,5,3,4 };
    static_vector<int> v4 = { 1,2,1,4 };

    REQUIRE(v1 == v2);
    REQUIRE(v1 != v3);
    REQUIRE(v1 < v3);
    REQUIRE(v1 <= v2);
    REQUIRE(v1 > v4);
    REQUIRE(v3 >= v4);
}