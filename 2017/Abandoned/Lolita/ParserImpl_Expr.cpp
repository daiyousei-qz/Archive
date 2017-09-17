#include "ParserImpl.h"
#include "ParserHelper.h"
#include "LiteralParser.h"
#include <string>
#include <vector>

using namespace std;

namespace lolita
{
	// Parsing Expr Impl
	//

	ExprBase* ParserImpl::ParseGenericSelection()
	{
		throw 0;
	}

	ExprBase* ParserImpl::ParsePrimaryExpr()
	{
		auto& tok = src_.Peek();

		if (src_.Try(TokenTag::Identifier))
		{
			return builder_.NewVariableExpr(tok.Content);
		}
		else if (src_.Try(TokenTag::IntegerConst))
		{
			return builder_.NewLiteralExpr(ParseIntegerConst(tok));
		}
		else if (src_.Try(TokenTag::FloatConst))
		{
			return builder_.NewLiteralExpr(ParseFloatConst(tok));
		}
		else if (src_.Try(TokenTag::CharConst))
		{
			return builder_.NewLiteralExpr(ParseCharConst(tok));
		}
		else if (src_.Try(TokenTag::StringLiteral))
		{
			return builder_.NewLiteralExpr(ParseStringLiteral(tok));
		}
		else if (src_.Try(TokenTag::Generic))
		{
			return ParseGenericSelection();
		}
		else if (src_.Try(TokenTag::LParenthesis))
		{
			ExprBase* result = ParseCommaExpr();
			ExpectToken(TokenTag::RParenthesis);
			return result;
		}

		ReportUnexpectedToken(src_.Peek());
		throw ParsingError{};
	}
	ExprBase* ParserImpl::ParseCompoundLiteral()
	{
		throw 0;
	}
	ExprBase* ParserImpl::ParsePostfixExpr()
	{
		if (src_.Try(TokenTag::RParenthesis))
			return ParseCompoundLiteral();

		auto expr = ParsePrimaryExpr();

		while (!src_.Exhausted())
		{
			if (src_.Try(TokenTag::LBracket))
			{
				// parsing array subscript
				auto index = ParseExpr();
				ExpectToken(TokenTag::RBracket);

				expr = builder_.NewSubscriptExpr(expr, index);
			}
			else if (src_.Try(TokenTag::LParenthesis))
			{
				// function invocation
				vector<ExprBase*> args;
				if (!src_.Try(TokenTag::RParenthesis))
				{
					// load arguments if non-empty
					do
					{
						args.push_back(ParseAssignmentExpr());

					} while (src_.Try(TokenTag::Comma));

					ExpectToken(TokenTag::RParenthesis);
				}

				expr = builder_.NewInvokeExpr(expr, std::move(args));
			}
			else if (src_.Try(TokenTag::Period))
			{
				// member access
				ExpectToken(TokenTag::Identifier);
				expr = builder_.NewAccessExpr(expr, src_.LastConsumed().Content, false);
			}
			else if (src_.Try(TokenTag::Arrow))
			{
				// ptr member access
				ExpectToken(TokenTag::Identifier);
				expr = builder_.NewAccessExpr(expr, src_.LastConsumed().Content, true);
			}
			else if (src_.Try(TokenTag::Increment))
			{
				expr = builder_.NewUnaryExpr(UnaryOp::BackInc, expr);
			}
			else if (src_.Try(TokenTag::Decrement))
			{
				expr = builder_.NewUnaryExpr(UnaryOp::BackDec, expr);
			}
			else
			{
				return expr;
			}
		}
	}
	ExprBase* ParserImpl::ParseUnaryExpr()
	{
		if (src_.Try(TokenTag::Increment))
		{
			return builder_.NewUnaryExpr(UnaryOp::FrontInc, ParseUnaryExpr());
		}
		else if (src_.Try(TokenTag::Decrement))
		{
			return builder_.NewUnaryExpr(UnaryOp::FrontDec, ParseUnaryExpr());
		}
		else if (src_.Try(TokenTag::Sizeof))
		{
			if (src_.Try(TokenTag::LParenthesis))
			{
				if (DetectExpression())
				{
					auto expr = ParseExpr();
					ExpectToken(TokenTag::RParenthesis);

					return builder_.NewSizeOfExpr(expr.Type);
				}
				else
				{
					auto type = ParseTypeName();
					ExpectToken(TokenTag::RParenthesis);

					return builder_.NewSizeOfExpr(type.Type);
				}
			}
			else
			{
				auto expr = ParseUnaryExpr();
				return builder_.NewSizeOfExpr(expr.type);
			}
		}
		else if (src_.Try(TokenTag::Alignof))
		{
			ExpectToken(TokenTag::LParenthesis);
			auto type = ParseTypeName();
			ExpectToken(TokenTag::RParenthesis);

			return builder_.NewAlignOfExpr(type.Type);
		}
		else
		{
			switch (src_.Peek().Tag)
			{
			case TokenTag::Ampersand:
			case TokenTag::Asterisk:
			case TokenTag::Plus:
			case TokenTag::Minus:
			case TokenTag::Tlide:
			case TokenTag::Exclamation:
			{
				auto tag = src_.Consume().Tag;
				return builder_.NewUnaryExpr(*TranslateUnaryOp(tag), ParseCastExpr());
			}
			default:
				return ParsePostfixExpr();
			}
		}
	}
	ExprBase* ParserImpl::ParseCastExpr()
	{
		if (src_.Try(TokenTag::LParenthesis))
		{
			auto type = ParseTypeName();
			ExpectToken(TokenTag::RParenthesis);
			auto expr = ParseCastExpr();

			return builder_.NewCastExpr(type, expr);
		}
		else
		{
			return ParseUnaryExpr();
		}
	}
	ExprBase* ParserImpl::ParseMultiplicativeExpr()
	{
		return ParseNaiveBinaryExpr<
			&ParserImpl::ParseCastExpr,
			TokenTag::Asterisk,
			TokenTag::Slash>();
	}
	ExprBase* ParserImpl::ParseAdditiveExpr()
	{
		return ParseNaiveBinaryExpr<
			&ParserImpl::ParseMultiplicativeExpr,
			TokenTag::Plus,
			TokenTag::Minus>();
	}
	ExprBase* ParserImpl::ParseShiftExpr()
	{
		return ParseNaiveBinaryExpr<
			&ParserImpl::ParseAdditiveExpr,
			TokenTag::LShift,
			TokenTag::RShift>();
	}
	ExprBase* ParserImpl::ParseRelationalExpr()
	{
		return ParseNaiveBinaryExpr<
			&ParserImpl::ParseShiftExpr,
			TokenTag::Less,
			TokenTag::LessEqual,
			TokenTag::Greater,
			TokenTag::GreaterEqual>();
	}
	ExprBase* ParserImpl::ParseEqualityExpr()
	{
		return ParseNaiveBinaryExpr<
			&ParserImpl::ParseRelationalExpr,
			TokenTag::Equal,
			TokenTag::Unequal>();
	}
	ExprBase* ParserImpl::ParseBitwiseAndExpr()
	{
		return ParseNaiveBinaryExpr<
			&ParserImpl::ParseEqualityExpr,
			TokenTag::Ampersand>();
	}
	ExprBase* ParserImpl::ParseBitwiseXorExpr()
	{
		return ParseNaiveBinaryExpr<
			&ParserImpl::ParseBitwiseAndExpr,
			TokenTag::Caret>();
	}
	ExprBase* ParserImpl::ParseBitwiseOrExpr()
	{
		return ParseNaiveBinaryExpr<
			&ParserImpl::ParseBitwiseXorExpr,
			TokenTag::Pipe>();
	}
	ExprBase* ParserImpl::ParseLogicalAndExpr()
	{
		return ParseNaiveBinaryExpr<
			&ParserImpl::ParseBitwiseOrExpr,
			TokenTag::DoubleAmp>();
	}
	ExprBase* ParserImpl::ParseLogicalOrExpr()
	{
		return ParseNaiveBinaryExpr<
			&ParserImpl::ParseLogicalAndExpr,
			TokenTag::DoublePipe>();
	}
	ExprBase* ParserImpl::ParseConditionalExpr()
	{
		ExprBase *maybe_condition = ParseLogicalOrExpr();
		if (src_.Try(TokenTag::Question))
		{
			ExprBase *positive_branch = ParseCommaExpr();
			ExpectToken(TokenTag::Colon);
			ExprBase *negative_branch = ParseConditionalExpr();

			return builder_.NewConditionalExpr(maybe_condition, positive_branch, negative_branch);
		}

		// not a conditional-expression
		return maybe_condition;
	}
	ExprBase* ParserImpl::ParseAssignmentExpr()
	{
		ExprBase *lhs = ParseConditionalExpr();

		auto assign_op = TranslateBinaryOp(src_.Peek().Tag);
		if (assign_op && IsAssignmentOp(*assign_op))
		{
			// consume the assign op
			src_.Consume();
			// parse rhs
			// NOTE must call ParseAssignmentExpr here for right associativity
			ExprBase *rhs = ParseAssignmentExpr();

			return builder_.NewBinaryExpr(*assign_op, lhs, rhs);
		}

		// not an assignment-expression
		return lhs;
	}
	ExprBase* ParserImpl::ParseCommaExpr()
	{
		vector<ExprBase*> v;
		do
		{
			v.push_back(ParseAssignmentExpr());

		} while (src_.Try(TokenTag::Comma));

		// create comma expression only if there're multiple expressions
		if (v.size() > 1)
		{
			return builder_.NewCommaExpr(move(v));
		}
		else
		{
			return v.back();
		}
	}
	ExprBase* ParserImpl::ParseExpr()
	{
		return ParseCommaExpr();
	}
}