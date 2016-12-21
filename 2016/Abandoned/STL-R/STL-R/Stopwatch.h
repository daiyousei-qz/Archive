#pragma once
#ifndef STLR_STOPWATCH_H
#define STLR_STOPWATCH_H

#include "Basic.h"

namespace eds
{
	class Stopwatch //: Uncopyable
	{
	public:
		Stopwatch();
		Stopwatch(Stopwatch &&) = default;

		void Start();
		void Stop();
		void Reset();
		void Restart();

		size_t GetElapsedMilliseconds();
		size_t GetElapsedSeconds();

	private:
		long long _startPoint = 0;
		long long _endPoint = 0;
	};
}

#endif // !STLR_STOPWATCH_H