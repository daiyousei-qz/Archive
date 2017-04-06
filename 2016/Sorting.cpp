#include <functional>
#include <map>
#include <vector>
#include <queue>
#include <chrono>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cassert>

//
// Helper
//

struct OpTrack
{
	size_t cmp_cnt = 0, move_cnt = 0;
};

OpTrack &operator+=(OpTrack &lhs, const OpTrack &rhs)
{
	lhs.cmp_cnt += rhs.cmp_cnt;
	lhs.move_cnt += rhs.move_cnt;

	return lhs;
}

//
// Sorting Impl
//

// at least bidirectional iter
template <typename TIter, typename TComp>
OpTrack InsertionSort(TIter begin, TIter end, TComp cmp)
{
	size_t cmp_cnt = 0, move_cnt = 0;

	for (auto boundary = std::next(begin); boundary != end; ++boundary)
	{
		auto cmp_with_cur = [&](const decltype(*boundary) &value) { return cmp(*boundary, value); };
		TIter p = std::find_if(begin, boundary, cmp_with_cur);

		cmp_cnt += std::distance(begin, p) + 1;
		move_cnt += std::distance(p, boundary);
		if (p != boundary)
		{
			auto tmp = std::move(*boundary);
			std::move_backward(p, boundary, std::next(boundary));
			*p = std::move(tmp);
		}
	}

	return OpTrack{ cmp_cnt, move_cnt };
}

// at least forward iter
template <typename TIter, typename TComp>
OpTrack Shellsort(TIter begin, TIter end, TComp cmp)
{
	size_t cmp_cnt = 0, move_cnt = 0;
	size_t seq_len = std::distance(begin, end);

	for (size_t gap = seq_len / 2; gap > 0; gap /= 2)
	{
		size_t offset = gap;
		TIter head = begin;
		TIter cur = std::next(begin, gap);
		while (cur != end)
		{
			head = (offset % gap) ? std::next(head) : begin;

			// perform generalized insertion sort in [head..cur..gap]
			TIter p = head;
			while (p != cur && cmp(*p, *cur))
			{
				std::advance(p, gap);
				cmp_cnt += 1;
			}
			cmp_cnt += 1;

			if (p != cur)
			{
				auto tmp = std::move(*cur);
				while (p != cur)
				{
					std::swap(tmp, *p);
					std::advance(p, gap);
					move_cnt += 3;
				}

				std::swap(tmp, *cur);
			}

			// update loop variables
			cur = std::next(cur);
			offset += 1;
		}
	}

	return OpTrack{ cmp_cnt, move_cnt };
}

// at least forward iter
template <typename TIter, typename TComp>
OpTrack BubbleSort(TIter begin, TIter end, TComp cmp)
{
	size_t cmp_cnt = 0, move_cnt = 0;
	bool finished = false;
	while (!finished)
	{
		bool exchanged = false;
		TIter p = begin, s = std::next(begin);
		while (s != end)
		{
			cmp_cnt += 1;
			if (cmp(*s, *p))
			{
				std::iter_swap(p, s);
				exchanged = true;

				move_cnt += 3;
			}

			++p;
			++s;
		}

		finished = !exchanged;
	}

	return OpTrack{ cmp_cnt, move_cnt };
}

// at least forward iter
template <typename TIter, typename TComp>
OpTrack QuickSort(TIter begin, TIter end, TComp cmp)
{
	OpTrack track;

	if (begin != end)
	{
		auto pivot = *std::next(begin, std::distance(begin, end) / 2);
		TIter mid1 = std::partition(begin, end, 
			[&](const auto& value) 
		{
			bool result = cmp(value, pivot);
			track.cmp_cnt += 1;
			track.move_cnt += result ? 0 : 3;

			return result;
		});
		TIter mid2 = std::partition(mid1, end, 
			[&](const auto& value) 
		{
			bool result = !cmp(pivot, value);
			track.cmp_cnt += 1;
			track.move_cnt += result ? 0 : 3;

			return result;
		});

		// NOTE actually mid2 = std::next(mid1) as *mid1 = pivot
		track += QuickSort(begin, mid1, cmp);
		track += QuickSort(mid2, end, cmp);
	}

	return track;
}

// at least forward iter
template <typename TIter, typename TComp>
OpTrack SelectionSort(TIter begin, TIter end, TComp cmp)
{
	size_t cmp_cnt = 0, move_cnt = 0;
	for (TIter cur = begin; cur != end; ++cur)
	{
		auto min_iter = std::min_element(cur, end, cmp);
		std::iter_swap(cur, min_iter);

		cmp_cnt += std::distance(cur, end) - 1;
		move_cnt += min_iter != cur ? 3 : 0;
	}

	return OpTrack{ cmp_cnt, move_cnt };
}

// at least random access iter
template <typename TIter, typename TComp>
OpTrack HeapSort(TIter begin, TIter end, TComp cmp)
{
	OpTrack track;
	auto cmp_helper = [&](const auto &lhs, const auto &rhs)
	{
		bool result = cmp(lhs, rhs);
		track.cmp_cnt += 1;
		track.move_cnt += result ? 0 : 3;

		return result;
	};

	std::make_heap(begin, end, cmp_helper);
	for (TIter boundary = end; boundary != begin; --boundary)
	{
		std::pop_heap(begin, boundary, cmp_helper);
	}

	return track;
}

// at least bidirectional iter
template <typename TIter, typename TComp>
OpTrack MergeSort(TIter begin, TIter end, TComp cmp)
{
	OpTrack track;
	size_t seq_len = std::distance(begin, end);

	if (seq_len > 1)
	{
		TIter mid = std::next(begin, seq_len / 2);

		track += MergeSort(begin, mid, cmp);
		track += MergeSort(mid, end, cmp);

		std::inplace_merge(begin, mid, end, 
			[&](const auto &lhs, const auto &rhs)
		{
			bool result = cmp(lhs, rhs);
			track.cmp_cnt += 1;
			track.move_cnt += result ? 0 : 3;

			return result;
		});

	}

	return track;
}

// acquire TElem to be integral and only less is supported
template <typename TIter>
void RadixSort(TIter begin, TIter end)
{
	using TElem = std::decay_t<decltype(*std::declval<TIter>())>;
	static_assert(std::is_integral_v<TElem>, "!");

	std::vector<TElem> buffer;
	std::vector<TElem> buckets[10];

	const size_t seq_len = std::distance(begin, end);
	std::copy(begin, end, std::back_inserter(buffer));
	int base = 10;

	bool done = false;
	while (!done)
	{
		for (int x : buffer)
		{
			size_t index = (x % base) / (base / 10);
			buckets[index].push_back(x);
		}

		done = buckets[0].size() == seq_len;

		auto iter = buffer.begin();
		for (std::vector<TElem> &v : buckets)
		{
			std::copy(v.begin(), v.end(), iter);
			std::advance(iter, v.size());

			v.clear();
		}

		base *= 10;
	}

	std::copy(buffer.begin(), buffer.end(), begin);
}

//
// Testbench
//

using microseconds_t = std::chrono::microseconds;

template <typename TIter, typename TComp>
using SortInvoker = OpTrack(*)(TIter begin, TIter end, TComp cmp);

static constexpr size_t TestRepetition = 3;

template <typename TElem, typename TInvoker>
auto TestComparativeSortingInternal(const std::vector<TElem> &data, TInvoker func)
{
	// verify the algorithm
	auto sample = data;
	auto cmp = std::less<TElem>();
	auto track = func(sample.begin(), sample.end(), cmp);
	if (!std::is_sorted(sample.begin(), sample.end(), cmp))
	{
		throw std::runtime_error("algorithm not working");
	}

	// evaluate the algorithm
	using clock = std::chrono::high_resolution_clock;
	std::vector<std::vector<TElem>> testbench(TestRepetition, data);

	auto first_tick = clock::now();
	for (size_t i = 0; i < TestRepetition; ++i)
	{
		auto &target = testbench[i];
		func(std::begin(target), std::end(target), cmp);
	}
	auto last_tick = clock::now();

	microseconds_t dur = std::chrono::duration_cast<microseconds_t>(last_tick - first_tick) / TestRepetition;
	return std::make_pair(track, dur);
}

void PrintComparativeTestResult(const char *promt, std::pair<OpTrack, microseconds_t> result)
{
	printf("%s %12lldus %12dcmp %12dmove\n",
		promt,
		result.second.count(),
		result.first.cmp_cnt, 
		result.first.move_cnt);
}

template <typename TElem, typename TInvoker>
void TestComparativeSorting(const std::map<std::string, TInvoker> &testees, const std::vector<TElem> &data)
{
	auto sorted = data;
	std::sort(sorted.begin(), sorted.end(), std::less<TElem>());
	auto rsorted = data;
	std::sort(rsorted.begin(), rsorted.end(), std::greater<TElem>());

	for (const auto &item : testees)
	{
		printf("==================================== %s ====================================\n", item.first.c_str());
		PrintComparativeTestResult("data in chaos            :", TestComparativeSortingInternal(data, item.second));
		PrintComparativeTestResult("data in ascending order  :", TestComparativeSortingInternal(sorted, item.second));
		PrintComparativeTestResult("data in descending order :", TestComparativeSortingInternal(rsorted, item.second));
		printf("\n");
	}
}

template <typename TElem>
auto TestRadixSortingInternal(const std::vector<TElem> &data)
{
	// verify the algorithm
	auto sample = data;
	RadixSort(sample.begin(), sample.end());
	if (!std::is_sorted(sample.begin(), sample.end(), std::less<TElem>()))
	{
		throw std::runtime_error("algorithm not working");
	}

	// evaluate the algorithm
	using clock = std::chrono::high_resolution_clock;
	std::vector<std::vector<TElem>> testbench(TestRepetition, data);

	auto first_tick = clock::now();
	for (size_t i = 0; i < TestRepetition; ++i)
	{
		auto &target = testbench[i];
		RadixSort(std::begin(target), std::end(target));
	}
	auto last_tick = clock::now();

	return std::chrono::duration_cast<microseconds_t>(last_tick - first_tick) / TestRepetition;
}

template <typename TElem>
void TestRadixSorting(const std::vector<TElem> &data)
{
	printf("==================================== RadixSort ====================================\n");
	printf("data in chaos            : %12lldus\n", TestRadixSortingInternal(data).count());
	printf("\n");
}

template <typename TElem = int,
		  typename TIter = decltype(std::declval<std::vector<TElem>>().begin()),
		  typename TInvoker = SortInvoker<TIter, std::less<TElem>>>
void DoTestSorting()
{
	std::vector<TElem> data;
	std::generate_n(std::back_inserter(data), 10000, std::rand);

	// Test comparative sorting
#define TO_INVOKER(FUNC_NAME) (&FUNC_NAME<TIter, std::less<TElem>>)
#define MAKE_TESTEE(FUNC_NAME) {(#FUNC_NAME), (TO_INVOKER(FUNC_NAME))}

	std::map<std::string, TInvoker> testees
	{
		MAKE_TESTEE(InsertionSort),
		MAKE_TESTEE(Shellsort),
		MAKE_TESTEE(BubbleSort),
		MAKE_TESTEE(QuickSort),
		MAKE_TESTEE(SelectionSort),
		MAKE_TESTEE(HeapSort),
		MAKE_TESTEE(MergeSort),
	};

	TestComparativeSorting<TElem, TInvoker>(testees, data);

#undef TO_INVOKER
#undef MAKE_TESTEE

	// Test non-comparative sorting
	TestRadixSorting(data);
}

int main()
{
	DoTestSorting();

	system("pause");
	return 0;
}

//
// possible implementation for important algorhithm
//
namespace detail
{
	// std::partition
	template <typename BidirIt, typename UnaryPredicate>
	BidirIt partition(BidirIt first, BidirIt last, UnaryPredicate p)
	{
		size_t seq_len = std::distance(first, last);
		BidirIt left = first;
		BidirIt right = std::prev(last);

		size_t counter = seq_len - 1;
		while (true)
		{
			while (p(*left))
			{
				++left;
				--counter;
			}
			while (!p(*right))
			{
				--right;
				--counter;
			}

			if (counter > 0)
			{
				std::iter_swap(left, right);

				++left;
				--right;
				counter -= 2;
			}
			else
			{
				return left;
			}
		}
	}
	// std::make_heap
	template <typename RandomIt, typename Compare>
	void make_heap(RandomIt first, RandomIt last, Compare cmp)
	{
		for (RandomIt boundary = std::next(first); boundary != last; ++boundary)
		{
			size_t i = std::distance(first, boundary);
			size_t copy = i;
			while (i > 0)
			{
				if (cmp(*std::next(first, i), *boundary))
				{
					i = (i - 1) / 2;
				}
			}

			if (i != copy)
			{
				std::iter_swap(boundary, std::next(first, i));
			}
		}
	}
	// std::pop_heap
	template <typename RandomIt, typename Compare>
	void pop_heap(RandomIt first, RandomIt last, Compare cmp)
	{
		size_t seq_len = std::distance(first, last);
		decltype(*first) tmp = std::move(*first);

		for (size_t i = 1; i < seq_len; i *= 2)
		{
			RandomIt left = std::next(first, i);
			RandomIt right = std::next(left);

			if (right != last && cmp(*left, *right))
			{
				i += 1; // mark index for child with larger content
			}

			size_t j = (i - 1) / 2; // index for parent
			std::iter_swap(std::next(first, j), std::next(first, i));
		}

		std::swap(tmp, *std::prev(last));
	}

	// std::inplace_merge
	template <typename BidirIt, typename Compare>
	void inplace_merge(BidirIt first, BidirIt middle, BidirIt last, Compare cmp)
	{
		BidirIt it1 = first;
		BidirIt it2 = middle;

		while (it1 != middle || it2 != last)
		{
			if (cmp(*it1, *it2))
			{
				++it1;
			}
			else
			{
				std::iter_swap(it1, it2);
				++it2;
			}
		}
	}

}