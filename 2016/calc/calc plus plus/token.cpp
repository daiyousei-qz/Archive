#include "token.h"
#include <string>

using std::shared_ptr;
using std::make_shared;
using std::string;

class Token_Operator : Token
{
	Operator _operator;

public:
	Token_Operator(const Operator &op) : Token(TokenType::Operator), _operator(op)
	{

	}

	virtual const Operator &GetOperator() const
	{
		return _operator;
	}
};

class Token_Operand : Token
{
	value_t _value;

public:
	Token_Operand(value_t val) : Token(TokenType::Operand), _value(val)
	{

	}

	virtual value_t GetValue() const
	{
		return _value;
	}
};

shared_ptr<Token> Token::NewPlaceholder()
{
	return make_shared<Token>();
}

shared_ptr<Token> Token::NewLeftParenthesis()
{
	return make_shared<Token>(TokenType::LeftParenthesis);
}

shared_ptr<Token> Token::NewRightParenthesis()
{
	return make_shared<Token>(TokenType::RightParenthesis);
}

shared_ptr<Token> Token::NewOperator(const Operator &op)
{
	auto token = new Token_Operator(op);
	return shared_ptr<Token>(reinterpret_cast<Token*>(token));
}

shared_ptr<Token> Token::NewOperand(value_t val)
{
	auto token = new Token_Operand(val);
	return shared_ptr<Token>(reinterpret_cast<Token*>(token));
}

TokenType Token::GetType() const
{
	return _type;
}

const Operator &Token::GetOperator() const
{
	throw string(" ");
}

value_t Token::GetValue() const
{
	throw string(" ");
}