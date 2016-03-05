#pragma once

#include <string>
#include <memory>
#include "operators.h"

enum class TokenType
{
	Placeholder,
	LeftParenthesis,
	RightParenthesis,
	Operator,
	Operand
};


class Token
{
	TokenType _type;

public:
	Token(TokenType type = TokenType::Placeholder) : _type(type) { }

	static std::shared_ptr<Token> NewPlaceholder();
	static std::shared_ptr<Token> NewLeftParenthesis();
	static std::shared_ptr<Token> NewRightParenthesis();
	static std::shared_ptr<Token> NewOperator(const Operator &op);
	static std::shared_ptr<Token> NewOperand(value_t val);

	TokenType GetType() const;

	virtual value_t GetValue() const;
	virtual const Operator &GetOperator() const;
};