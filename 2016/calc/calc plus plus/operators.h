#pragma once

#include <string>

using value_t = double;
using level_t = unsigned int;

using OperatorDelegate = value_t(*)(value_t lhs, value_t rhs);

enum class OperatorAssociativity
{
	Left,
	Right
};

struct Operator
{
	const std::string Symbol;
	const level_t Precedence;
	const OperatorAssociativity Associativity;
	const OperatorDelegate Delegate;
};

const Operator &Lookup(const std::string &&symbol);