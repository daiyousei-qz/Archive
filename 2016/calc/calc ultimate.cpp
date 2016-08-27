#include <map>
#include <deque>
#include <string>
#include <cassert>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <functional>

struct element_t
{
	char op;
	int value;
};

bool is_op(char ch)
{
	static char ops[] = { '+', '-', '*', '/', '(', ')' };
	return std::find(std::begin(ops), std::end(ops), ch) != std::end(ops);
}

std::deque<element_t> parse(const std::string& expr)
{
	std::deque<element_t> midfix;
	std::istringstream ss{ expr };
	while (!ss.eof())
	{
		int val;
		char op;

		// operands
		if (ss >> val)
		{
			midfix.push_back({ 0, val });
		}
		else
		{
			ss.clear();
		}

		// operators
		if (ss >> op)
		{
			assert(is_op(op));
			midfix.push_back({ op, 0 });
		}
	}

	return midfix;
}

int get_precedence(char op)
{
	if (op == '+' || op == '-')
		return 1;
	if (op == '*' || op == '/')
		return 2;
	if (op == '(' || op == ')')
		return 3;

	// invalid op
	assert(false);
}

std::deque<element_t> to_postfix(std::deque<element_t> midfix)
{
	std::deque<element_t> op_buf;
	std::deque<element_t> postfix;

	while (!midfix.empty())
	{
		// retrive first element
		element_t elem = midfix.front();
		midfix.pop_front();

		if (elem.op == 0)
		{
			// then it's a value
			postfix.push_back(elem);
		}
		else
		{
			// then it's an op
			if (elem.op == ')')
			{
				// jungle RP')' without a opening LR'('
				assert(!op_buf.empty());

				// output ops until LP'(' is hit
				while (op_buf.back().op != '(')
				{
					postfix.push_back(op_buf.back());
					op_buf.pop_back();

					// jungle RP')' without a opening LR'('
					assert(!op_buf.empty());
				}

				// pop LR'('
				op_buf.pop_back();
			}
			else
			{
				// also includes LR'(' as it has the highest precedence
				// but LP'(' won't be discarded until RP')' is hit
				while (!op_buf.empty() &&
						op_buf.back().op != '(' &&
						get_precedence(op_buf.back().op) >= get_precedence(elem.op))
				{
					postfix.push_back(op_buf.back());
					op_buf.pop_back();
				}

				op_buf.push_back(elem);
			}
		}
	}

	// jungle LR'(' without a closing RP')'
	assert(std::find_if(op_buf.begin(), op_buf.end(), 
						[](auto elem) { return elem.op == '('; }) == op_buf.end());

	// push remaining ops
	postfix.insert(postfix.end(), op_buf.rbegin(), op_buf.rend());

	return postfix;
}

int calc(std::deque<element_t> postfix)
{
	using delegate_t = std::function<int(int, int)>;
	static std::map<char, delegate_t> op_map
	{
		{ '+', std::plus<int>{} },
		{ '-', std::minus<int>{} },
		{ '*', std::multiplies<int>{} },
		{ '/', std::divides<int>{} }
	};

	std::deque<int> tmp;
	while (!postfix.empty())
	{
		// retrive top element
		element_t elem = postfix.front();
		postfix.pop_front();

		// evaluate
		if (elem.op == 0)
		{
			// then it's a value
			tmp.push_back(elem.value);
		}
		else
		{
			// then it's an op
			assert(tmp.size() > 1);

			int t1 = tmp.back();
			tmp.pop_back();
			int t2 = tmp.back();
			tmp.pop_back();

			tmp.push_back(op_map[elem.op](t2, t1));
		}
	}

	assert(tmp.size() == 1);
	return tmp.back();
}

int eval(const std::string& expr)
{
	auto midfix = parse(expr);
	auto postfix = to_postfix(std::move(midfix));

	return calc(std::move(postfix));
}

int main()
{
	int val = eval("(2+3)*(  4)");
	return 0;
}