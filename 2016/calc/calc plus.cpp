// warning: hard-coded way to solve precedence

#include <iostream>
#include <sstream>
#include <string>
#include <list>

using namespace std;

struct node
{
	bool is_operator;
	int value;
};

// only +/- remaining in stack
int calc3(list<node> stk)
{
	int tmp = stk.front().value;
	stk.pop_front();
	while(!stk.empty())
	{
		// according to associativity, calculate step by step
		int op = stk.front().value;
		stk.pop_front();
		int r = stk.front().value;
		stk.pop_front();

		switch (op)
		{
		case '+':
			tmp += r;
			break;
		case '-':
			tmp -= r;
			break;
		default:
			throw std::runtime_error("unexpected operator.");
		}
	}

	return tmp;
}

// stack without parenthesis
int calc2(list<node> stk)
{
	list<node> tmp;
	tmp.push_back(stk.front());
	stk.pop_front();

	while (!stk.empty())
	{
		// calculate product and division only
		int op = stk.front().value;
		stk.pop_front();
		int r = stk.front().value;
		stk.pop_front();

		int x;
		switch (op)
		{
		case '+':
		case '-':
			tmp.push_back({ true, op });
			tmp.push_back({ false, r });
			break;
		case '*':
			x = tmp.back().value;
			tmp.pop_back();
			tmp.push_back({ false, x * r });
			break;
		case '/':
			x = tmp.back().value;
			tmp.pop_back();
			tmp.push_back({ false, x / r });
			break;
		default:
			throw std::runtime_error("unexpected operator.");
		}
	}

	return calc3(tmp);
}

int calc1(list<node> stk)
{
	for (auto iter = stk.cbegin(); iter != stk.cend(); )
	{
		if (iter->is_operator && iter->value == ')')
		{
			auto bak = ++iter;
			// trace back to find coresponding (
			while (iter != stk.cbegin() && iter->value != '(') --iter;

			if (iter->is_operator && iter->value == '(')
			{
				list<node> tmp(iter, bak);
				tmp.pop_front();
				tmp.pop_back();
				iter = stk.erase(iter, bak);
				iter = stk.insert(iter, { false, calc2(tmp) });
			}
			else
			{
				throw runtime_error("unclosed parenthesis");
			}
		}
		else
		{
			++iter;
		}
	}

	// there should be no ( left
	for (const auto &node : stk)
	{
		if(node.is_operator && node.value == '(')
			throw runtime_error("unclosed parenthesis");
	}

	return calc2(stk);
}

int main()
{
	//list<node> test = { {false, 8}, {true, '+'}, {true, '('},{false, 5}, {true, '+'}, {false, 2}, {true, '*'}, {false, 2}, { true, ')'} };
	//auto result = calc1(test);

	string input;
	while (1)
	{
		cout << "Please input expression or \"end\" to quit." << endl;
		getline(cin, input);

		if (input == "end")
		{
			break;
		}
		else
		{
			istringstream ss(input);
			list<node> stk;

			while (!ss.eof())
			{
				int val;
				char op;

				if (ss >> val)
					stk.push_back({ false, val });
				else
					ss.clear();

				if (ss >> op)
					stk.push_back({ true, op });
			}

			try
			{
				cout << calc1(stk) << endl;
			}
			catch (const runtime_error &ex)
			{
				cerr << ex.what() << endl;
			}
		}
	}

	return 0;
}