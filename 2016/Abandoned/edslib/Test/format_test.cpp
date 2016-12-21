#include "catch.hpp"
#include "../Source/format.hpp"
#include <string>

TEST_CASE("format::")
{
	using namespace eds;
	
	{
		std::string expected = "a(1,2.2,3,\"4\")";
		std::string yield = Format("a({},{},{},{})", 1, 2.2, '3', "\"4\"");

		REQUIRE(expected == yield);
	}

	{
		std::string expected = "{text}";
		std::string yield = Format("{{{0}}}", "text");

		REQUIRE(expected == yield);
	}

	{
		std::string expected = "test-332211";
		std::string yield = Format("test-{2}{1}{0}", 11, 22, 33);

		REQUIRE(expected == yield);
	}

	{
		std::string expected = "112211!!";
		std::string yield = Format("{}{}{0}!!", 11, 22);

		REQUIRE(expected == yield);
	}

	{
		std::wstring expected = L"a{1,2.2,3,\"4\"}";
		std::wstring yield = Format(L"a{{{},{},{},{}}}", 1, 2.2, L'3', L"\"4\"");

		REQUIRE(expected == yield);
	}
}