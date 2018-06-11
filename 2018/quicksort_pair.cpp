#include <algorithm>
#include <iterator>
#include <random>

// PROBLEM:
// given two arrays of floating number, consists of the same set of float, but in dufferent order,
// please calculate index mapping of the same number without comparison of two elements in the same array
// NOTE you may compare a number from A with one from B

using namespace std;

int CenteralizePivot(double value, double* dest, int* destPos, int n)
{
	int i = 0;
	int j = n - 1;

	while (i < j)
	{
		while (dest[i] < value)
			i += 1;

		while (dest[j] > value)
			j -= 1;

		swap(dest[i], dest[j]);
		swap(destPos[i], destPos[j]);
	}

	return i;
}

void PairSort(double* a, int* ap, double* b, int* bp, int n)
{
	if (n < 2)
		return;

	int i = CenteralizePivot(*a, b, bp, n);
	CenteralizePivot(b[i], a, ap, n);

	PairSort(a, ap, b, bp, i);
	PairSort(a+i+1, ap+i+1, b+i+1, bp+i+1, n-i-1);
}

int main()
{
	// initialize
	constexpr int n = 100;

	double a[n] = {};
	double b[n] = {};

	int ap[n] = {};
	int bp[n] = {};

	for (int i = 0; i < n; ++i)
	{
		a[i] = b[i] = i * 1.0;
		ap[i] = bp[i] = i;
	}

	auto prng = std::default_random_engine{};
	shuffle(begin(a), end(a), prng);
	shuffle(begin(b), end(b), prng);

	// do pair sorting
	PairSort(a, ap, b, bp, n);

	// print result
	for (int i = 0; i < n; ++i)
	{
		printf("[%d - %d] %f\n", ap[i], bp[i], a[i]);
	}

	system("pause");
}