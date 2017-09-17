#pragma once

namespace lolita
{
	class NonCopyable
	{
	public:
		NonCopyable() = default;

		NonCopyable(const NonCopyable&) = delete;
		NonCopyable& operator=(const NonCopyable&) = delete;

		NonCopyable(NonCopyable&&) = default;
		NonCopyable& operator=(NonCopyable&&) = default;
	};

	class NonMovable
	{
	public:
		NonMovable() = default;

		NonMovable(const NonMovable&) = delete;
		NonMovable& operator=(const NonMovable&) = delete;

		NonMovable(NonMovable&&) = delete;
		NonMovable& operator=(NonMovable&&) = delete;
	};
}