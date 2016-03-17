#pragma once
//============================================================
// EdsLab Math
//============================================================
// File:   math.h
// Author: Edward Cheng
// Desc:   Math utilities.
//============================================================

namespace math
{

	constexpr unsigned FACTORIAL(unsigned n) { return n > 1 ? (n * FACTORIAL(n-1)) : 1; }

	constexpr double PI = 3.141592653589793;
	constexpr double E = 2.718281828459045;

	// |x|
	double abs(double x)
	{
		return x > 0 ? x : -x;
	}

	// [x]
	double floor(double x)
	{
		// temporary implementation
		return static_cast<long long>(x);
	}

	double ln(double x)
	{
		if (x <= 0) throw 0;

		double factor = 0;
		while (x > 2)
		{
			++factor;
			x /= E;
		}

		double val = x - 1;
		double sum = 0;

		int sign = 1;
		double pow = val;
		for (int i = 0; i < 100; ++i)
		{
			double tmp = pow / (i + 1);

			sum += tmp * sign;
			sign = -sign;
			pow *= val;
		}

		return sum + factor;
	}

	double power(double x, int y)
	{
		if (y == 0) return 1;
		if (y < 0) return 1 / power(x, -y);

		auto tmp = power(x, y / 2);
		return tmp * tmp * (y & 1 ? x : 1);
	}

	// x**y
	double power(double x, double y)
	{
		double bound = floor(y);
		double delta = y - bound;
		double base = power(x, (int)bound);
		double lnx = ln(x);

		double sum = 0;
		for (int i = 0; i < 100; ++i)
		{
			double tmp = base;
			for (int j = 0; j < i; ++j)
				tmp = tmp * lnx * delta / (j + 1);
			
			sum += tmp;
		}

		return bound + sum;
	}

	double sqrt(double x)
	{
		return power(x, 0.5);
	}
}