#include "Stopwatch.h"
#include "windows.h"

namespace eds
{
	static long long SYSTEM_TICKER_FREQUENCY = 0;

	Stopwatch::Stopwatch()
	{
		if (SYSTEM_TICKER_FREQUENCY == 0)
		{
			QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&SYSTEM_TICKER_FREQUENCY));
		}
	}

	void Stopwatch::Start()
	{
		if (_startPoint == 0)
		{
			QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&_startPoint));
		}
	}

	void Stopwatch::Stop()
	{
		if (_endPoint == 0)
		{
			QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&_endPoint));
		}
	}

	void Stopwatch::Reset()
	{
		_startPoint = _endPoint = 0;
	}

	void Stopwatch::Restart()
	{
		Reset();
		Start();
	}

	size_t Stopwatch::GetElapsedMilliseconds()
	{
		if (_startPoint == 0) return 0;

		long long end = _endPoint;
		if (end == 0)
		{
			QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&end));
		}

		return static_cast<size_t>((end - _startPoint) * 1000 / SYSTEM_TICKER_FREQUENCY);
	}

	size_t Stopwatch::GetElapsedSeconds()
	{
		return GetElapsedMilliseconds() / 1000;
	}
}