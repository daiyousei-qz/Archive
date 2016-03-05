// I'm going to implement an generic expression evaluator that evaluate with general binary left associative operators.

#include "token.h"
#include "operators.h"
#include <iostream>
#include <sstream>
#include <list>
#include <string>
#include <memory>

using std::string;
using std::list;
using std::weak_ptr;
using std::shared_ptr;

struct TreeNode
{
	const shared_ptr<Token> Content;

	weak_ptr<TreeNode> Parent;
	shared_ptr<TreeNode> LeftChild;
	shared_ptr<TreeNode> RightChild;

	TreeNode(shared_ptr<Token> token) : Content(token)
	{
		// empty
	}

	static void MakeLeftChild(shared_ptr<TreeNode> parent, shared_ptr<TreeNode> node)
	{
		parent->LeftChild = node;
		node->Parent = parent;
	}

	static void MakeRightChild(shared_ptr<TreeNode> parent, shared_ptr<TreeNode> node)
	{
		parent->RightChild = node;
		node->Parent = parent;
	}
};

class ExpEvaluator
{
	using token_list = list<shared_ptr<Token>>;
	using iterator = token_list::iterator;
	
	value_t _EvaluateNode(shared_ptr<TreeNode> node)
	{
		auto node_type = node->Content->GetType();
		if (node_type == TokenType::Operand)
		{
			return node->Content->GetValue();
		}
		else
		{
			auto lhs = _EvaluateNode(node->LeftChild);
			auto rhs = _EvaluateNode(node->RightChild);
			return node->Content->GetOperator().Delegate(lhs, rhs);
		}
	}

	// Associativity has not been considered
	value_t _Evaluate_Unparenthesed(iterator begin, iterator end)
	{
		if (begin == end) throw string("bad expression");
		auto root = std::make_shared<TreeNode>(*begin);
		auto cur = root;
		for (auto iter = ++begin; iter != end; ++iter)
		{
			auto node = std::make_shared<TreeNode>(*iter);
			auto node_type = node->Content->GetType();
			if (node_type == TokenType::Operand)
			{
				// insert operand if needed
				if (cur->Content->GetType() == TokenType::Operator
					&& !cur->RightChild)
				{
					TreeNode::MakeRightChild(cur, node);
				}
				else
				{
					// format error
					throw string("repeated operand.");
				}
			}
			else if (node_type == TokenType::Operator)
			{
				if (cur->Content->GetType() == TokenType::Operator)
				{
					auto &op = node->Content->GetOperator();
					// find where to insert operator
					while (op.Precedence <= cur->Content->GetOperator().Precedence && cur != root)
						cur = cur->Parent.lock();

					if (op.Precedence <= cur->Content->GetOperator().Precedence)
					{
						if (cur != root)
						{
							TreeNode::MakeRightChild(cur->Parent.lock(), node);
						}
						else
						{
							root = node;
						}

						TreeNode::MakeLeftChild(node, cur);
					}
					else
					{
						// insert node to the tree
						TreeNode::MakeLeftChild(node, cur->RightChild);
						TreeNode::MakeRightChild(cur, node);
					}
				}
				else if(cur->Content->GetType() == TokenType::Operand)
				{
					TreeNode::MakeLeftChild(node, cur);
					root = node;
				}
				else
				{
					throw string(" ");
				}

				cur = node;
			}
			else
			{
				throw string("unexpected token.");
			}
		}

		// now we has an Evaluation Tree at root
		return _EvaluateNode(root);
	}

	value_t _Evaluate(token_list exp)
	{
		for (iterator iter = exp.begin(); iter != exp.end(); ++iter)
		{
			if ((*iter)->GetType() == TokenType::RightParenthesis)
			{
				// move iter forward to include ')'
				auto coresponding = iter++;
				// search backward to find coresponding '('
				while (!((*coresponding)->GetType() ==  TokenType::LeftParenthesis) && coresponding != exp.begin())
					--coresponding;
				// if found
				if ((*coresponding)->GetType() == TokenType::LeftParenthesis)
				{
					// insert a placeholder before parenthesed expression to record the place
					exp.insert(coresponding, Token::NewPlaceholder());
					// transfer the unparenthesed expression to another list
					token_list subexp;
					subexp.splice(subexp.begin(), exp, coresponding, iter);
					// remove parenthesis from head and tail
					subexp.pop_front();
					subexp.pop_back();
					// replace the placeholder with evaluated expression value
					for (iter = exp.begin(); !((*iter)->GetType() == TokenType::Placeholder); ++iter);
					auto value = _Evaluate_Unparenthesed(subexp.begin(), subexp.end());
					*iter = Token::NewOperand(value);
				}
				else
				{
					throw string("unmatched parenthesis.");
				}
			}
		}

		return _Evaluate_Unparenthesed(exp.begin(), exp.end());
	}

public:
	value_t Evaluate(string exp)
	{
		std::istringstream ss(exp);
		token_list tokens;

		while (!ss.eof())
		{
			value_t val;
			char op;

			if (ss >> val)
				tokens.push_back(Token::NewOperand(val));
			else
				ss.clear();

			if (ss >> op)
			{
				switch (op)
				{
				case '(':
					tokens.push_back(Token::NewLeftParenthesis());
					break;
				case ')':
					tokens.push_back(Token::NewRightParenthesis());
					break;
				default:
					char tmp[] = { op, '\0' };
					tokens.push_back(Token::NewOperator(Lookup(tmp)));
					break;
				}
			}
		}

		return _Evaluate(tokens);
	}

	value_t Test()
	{
		// test -> 1+(1+2*3)*2 = 15
		token_list test = 
		{
			Token::NewOperand(1),
			Token::NewOperator(Lookup("+")),
			Token::NewLeftParenthesis(),
			Token::NewOperand(1),
			Token::NewOperator(Lookup("+")),
			Token::NewOperand(2),
			Token::NewOperator(Lookup("*")),
			Token::NewOperand(3),
			Token::NewRightParenthesis(),
			Token::NewOperator(Lookup("*")),
			Token::NewOperand(2),
		};

		return _Evaluate(test);
	}
};

int main() 
{
	ExpEvaluator eval;
	while (1)
	{
		string line;
		std::getline(std::cin, line);
		std::cout << eval.Evaluate(line) << std::endl;
	}

	return 0;
}