#include "operators.h"
#include <string>
#include <map>

using std::string;
using std::map;

Operator addition =
{
	"+",
	1,
	OperatorAssociativity::Left,
	[](auto lhs, auto rhs) { return lhs + rhs; }
};

Operator subtraction =
{
	"-",
	1,
	OperatorAssociativity::Left,
	[](auto lhs, auto rhs) { return lhs - rhs; }
};

Operator multiplication =
{
	"*",
	2,
	OperatorAssociativity::Left,
	[](auto lhs, auto rhs) { return lhs * rhs; }
};

Operator division =
{
	"/",
	2,
	OperatorAssociativity::Left,
	[](auto lhs, auto rhs) { return lhs / rhs; }
};

map<string, Operator> _lookup
{
	{ addition.Symbol, addition },
	{ subtraction.Symbol, subtraction },
	{ multiplication.Symbol, multiplication },
	{ division.Symbol, division }
};

//*
const Operator &Lookup(const string &&symbol)
{
	return _lookup.at(symbol);
}
//*/