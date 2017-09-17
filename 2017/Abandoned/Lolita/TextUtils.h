#pragma once
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <string_view>
#include <algorithm>

namespace lolita
{
	template <typename ... TArgs>
	inline std::string FormatString(const char *fmt, TArgs&& ...args)
	{
		char buf[2048];
		sprintf(buf, fmt, std::forward<TArgs>(args)...);

		return std::string(buf);
	}

	inline std::string Quote(const std::string &s)
	{
		std::stringstream ss;
		ss << std::quoted(s);

		return ss.str();
	}

	inline char ConsumeFront(std::string_view &view)
	{
		assert(!view.empty());

		auto ch = view.front();
		view.remove_prefix(1);

		return ch;
	}

	inline bool TestPrefix(const std::string_view &view, char ch)
	{
		return (!view.empty() && view.front() == ch);
	}

	inline bool TestAnyPrefix(const std::string_view &view, std::string_view any)
	{
		if (view.empty())
			return false;

		return std::find(any.begin(), any.end(), view.front()) != any.end();
	}

	inline bool TestSeqPrefix(const std::string_view &view, std::string_view seq)
	{
		if (view.length() < seq.length())
			return false;

		auto result = std::mismatch(seq.begin(), seq.end(), view.begin());
		return result.first == seq.end();
	}

	inline bool TryPrefix(std::string_view &view, char ch)
	{
		if (TestPrefix(view, ch))
		{
			view.remove_prefix(1);
			return true;
		}

		return false;
	}

	inline bool TryAnyPrefix(std::string_view &view, std::string_view any)
	{
		if (TestAnyPrefix(view, any))
		{
			view.remove_prefix(1);
			return true;
		}

		return false;
	}

	inline bool TrySeqPrefix(std::string_view &view, std::string_view seq)
	{
		if (TestSeqPrefix(view, seq))
		{
			view.remove_prefix(seq.length());
			return true;
		}

		return false;
	}
}