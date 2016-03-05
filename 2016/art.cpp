#include <vector>
#include <functional>

using v_int = std::vector<int>;
int sum(int lhs, int rhs) { return lhs + rhs; }
int pdt(int lhs, int rhs) { return lhs * rhs; }

int clac(const v_int &list, int (*op)(int, int))
{
	int s = 0, f = 1;
	for (auto x : list)
		s = f ? (f = 0, x) : op(s, x);

	return s;
}

auto wrap(int(*op)(int, int))  -> std::funcion<int(int, int)>
{
	return [op](const v_int &list) { return clac(list, op); };
}

int main()
{
	std::vector<int> v = { 1,2,3,4 };
	auto sum_s = wrap(sum), pdt_s = wrap(pdt);
	int s = sum_s(v);
	int p = pdt_s(v);

	return 0;
}