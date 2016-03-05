#include <iostream>
#include <map>
#include <stdexcept>

using pf = int(*)(const int, const int);
std::map<int, pf> ops;

void init()
{
	using std::pair;
	ops.insert({ 
	{ '+', [](int x, int y) { return x + y; } },
	{ '-', [](int x, int y) { return x - y; } },
	{ '*', [](int x, int y) { return x * y; } },
	{ '/', [](int x, int y) { if (y) return x / y; else throw std::runtime_error("Denominator cannot be zero."); } },
	});
}

void input(int &x, int &y, int &op)
{
	using namespace std;

	while (1)
	{
		char tmp;
		if (!(cin >> x)) break;
		if (!(cin >> tmp)) break;
		if (!(cin >> y)) break;
		op = tmp;
		return;
	}

	cin.clear();
	cin.seekg(ios::end);
	throw std::runtime_error("Invalid expression.");
}

int main()
{
	using namespace std;
	init();

	while (1)
	{
		cout << "Input an expression to claculate:" << endl;

		try
		{
			int x, y, op;
			input(x, y, op);

			const auto &pf = ops[op];
			if (pf != nullptr)
				cout << pf(x, y) << endl;
			else
				throw std::runtime_error("Unknown operator.");
		}
		catch (const runtime_error &ex)
		{
			cerr << ex.what() << endl;
		}
	}

	return 0;
}