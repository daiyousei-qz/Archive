#include "ParserImpl.h"
#include "ParserHelper.h"
#include "LiteralParser.h"
#include <string>
#include <vector>

using namespace std;

namespace lolita
{
	// Parsing Type Impl
	//

	bool ParserImpl::TryAppendQual(TypeQualifier& quals)
	{
		const auto& tok = src_.Peek();
		switch (tok.Tag)
		{
		case TokenTag::Const:
			quals.Const = true;
			break;
		case TokenTag::Restrict:
			quals.Restrict = true;
			break;
		case TokenTag::Volatile:
			quals.Volatile = true;
			break;
		case TokenTag::Atomic:
			throw 0;
			break;

		default:
			return false;
		}

		src_.Consume();
		return true;
	}

	bool ParserImpl::TryAppendFuncSpec(FunctionSpecifier& spec)
	{
		const auto& tok = src_.Peek();
		switch (tok.Tag)
		{
		case TokenTag::Inline:
			if (spec.Inline)
				ReportError(tok.Location, "redundant inline");
			spec.Inline = true;
			break;
		case TokenTag::Noreturn:
			if (spec.Noreturn)
				ReportError(tok.Location, "redundant noreturn");
			spec.Noreturn = true;
			break;

		default:
			return false;
		}

		src_.Consume();
		return true;
	}

	bool ParserImpl::TryAppendStorageSpec(StorageSpecifier& spec)
	{
		// parse token
		auto tok_spec = StorageSpecifier::None;
		switch (src_.Peek().Tag)
		{
		case TokenTag::Typedef:
			tok_spec = StorageSpecifier::Typedef;
			break;
		case TokenTag::Extern:
			tok_spec = StorageSpecifier::Extern;
			break;
		case TokenTag::Static:
			tok_spec = StorageSpecifier::Static;
			break;
		case TokenTag::Auto:
			tok_spec = StorageSpecifier::Auto;
			break;
		case TokenTag::Register:
			tok_spec = StorageSpecifier::Register;
			break;
		default:
			return false;
		}

		// consume the token anyway
		const auto& tok = src_.Consume();
		if (spec != StorageSpecifier::None)
			ReportError(tok.Location, "redundant storage spec");
		else
			spec = tok_spec;

		return true;
	}

	bool ParserImpl::TryAppendAlignas(size_t& align)
	{
		if (src_.Try(TokenTag::Alignas))
		{
			throw 0;
		}
		else
		{
			return false;
		}
	}

	QualType ParserImpl::ParseDeclSpec(
		FunctionSpecifier* func_spec,
		StorageSpecifier* storage_spec,
		bool* thd_local_spec,
		size_t* align_spec)
	{
		auto type = TypeSpecContext{};
		auto quals = TypeQualifier{};
		auto func = FunctionSpecifier{};
		auto storage = StorageSpecifier::None;
		bool thd_local = false;
		size_t align = 0u;

		while (true)
		{
			const auto& tok = src_.Peek();
			if (TryAppendQual(quals))
			{
				// do nothing
			}
			else if (TryAppendFuncSpec(func))
			{
				if (!func_spec)
					ReportError(tok.Location, "function specifier not expected");
			}
			else if (TryAppendStorageSpec(storage))
			{
				if (!storage_spec)
					ReportError(tok.Location, "storage specifier not expected");
			}
			else if (TryAppendAlignas(align))
			{
				if (!align_spec)
					ReportError(tok.Location, "alignment specifier not expected");
			}
			else if (src_.Try(TokenTag::ThreadLocal))
			{
				if (thd_local_spec)
				{
					if (thd_local)
						ReportError(tok.Location, "redundant _Thread_local");

					thd_local = true;
				}
				else
				{
					ReportError(tok.Location, "storage specifier not expected");
				}
			}
			else if (src_.Try(TokenTag::Struct))
			{
				throw 0;
			}
			else if (src_.Try(TokenTag::Union))
			{
				throw 0;
			}
			else if (src_.Try(TokenTag::Enum))
			{
				throw 0;
			}
			else
			{
				auto ps = initializer_list<TokenTag>
				{
					// TokenTag::Bool
					TokenTag::Void,
					TokenTag::Char, TokenTag::Int,
					TokenTag::Short, TokenTag::Long,
					TokenTag::Signed, TokenTag::Unsigned,
					TokenTag::Float, TokenTag::Double,
				};
				auto iter = find(ps.begin(), ps.end(), tok.Tag);
				if (iter != ps.end())
				{
					src_.Consume();
					type.Feed(*iter);
				}
				else
				{
					break;
				}
			}
		}

		if (func_spec)
			*func_spec = func;
		if (storage_spec)
			*storage_spec = storage;
		if (thd_local_spec)
			*thd_local_spec = thd_local;
		if (align_spec)
			*align_spec = align;

		// FIXME: add non-builtin type support
		if (type.IsBuiltinType())
			return GetTypeTable().MakeBuiltin(type.Export())->MakeQualified(quals);
		else
			throw 0;
	}

	TypeQualifier ParserImpl::ParseTypeQualifierList()
	{
		auto quals = TypeQualifier{};
		while (TryAppendQual(quals)) {}

		return quals;
	}


	QualType ParserImpl::ParsePointer(QualType type)
	{
		auto result = type;
		while (src_.Try(TokenTag::Asterisk))
		{
			auto ptr_type = GetTypeTable().MakePointer(result);
			auto quals = ParseTypeQualifierList();

			result = ptr_type->MakeQualified(quals);
		}

		return result;
	}

	// FIXME: only top-level parameter names would be annotated
	DeclaratorInfo ParserImpl::ParseDeclarator(QualType base, StringVec* param_annot)
	{
		// parse ptr definition
		auto name = std::optional<std::string>();
		auto base_type = ParsePointer(base);
		auto decl_type = base_type;
		auto maybe_incorrect_base = false;

		if (src_.Try(TokenTag::Identifier))
		{
			// declarator name found
			name = src_.LastConsumed().Content;
		}
		else if (src_.Try(TokenTag::LParenthesis))
		{
			// this can be another nested declarator or a parameter declaration

			// try nested declarator first
			// NOTE qualifier won't change anyway, if the base type doesn't change
			std::tie(decl_type, name) = ParseDeclarator(base_type, nullptr);
			if (decl_type.Type != base_type.Type)
			{
				// it's a nested declarator, and successfully parsed
				ExpectToken(TokenTag::RParenthesis);

				// now we may have wrong base type if there's array or function definition
				// we may have to correct that
				maybe_incorrect_base = true;
			}
			else
			{
				// it's not a nested declarator but a parameter list
				return std::make_pair(ParseFuncDecl(base_type, param_annot), name);
			}
		}
		
		QualType correct_base = base_type;
		if (src_.Try(TokenTag::LBracket))
		{
			// base should change to an array
			correct_base = ParseArrayDecl(base_type);
		}
		else if (src_.Try(TokenTag::LParenthesis))
		{
			// base should change to an function
			correct_base = ParseFuncDecl(base_type, param_annot);
		}
		else
		{
			// base is correct
			return std::make_pair(decl_type, name);
		}

		if (maybe_incorrect_base)
		{
			return std::make_pair(
				decl_type.Type
					->ShiftBase(base_type.Type, correct_base.Type)
					->MakeQualified(decl_type.Quals),
				name
			);
		}
		else
		{
			// FIXME: explain this
			// for no nested declarator?
			return std::make_pair(correct_base, name);
		}
	}

	QualType ParserImpl::ParseFuncDecl(QualType ret_type, StringVec* param_annot)
	{
		assert(src_.LastConsumed().Tag == TokenTag::LParenthesis);
		// FIXME: support foo(void)
		if (src_.Try(TokenTag::RParenthesis))
		{
			// empty parameter list
			return GetTypeTable().MakeFunction(ret_type, {}, false)->MakeQualified();
		}


		bool va_args = false;
		QualTypeVec param_types;
		do 
		{
			if (src_.Try(TokenTag::Ellipsis))
			{
				va_args = true;
				break;
			}

			auto base_type = ParseDeclSpec(nullptr, nullptr, nullptr, nullptr);
			auto decl_type = ParseDeclarator(base_type, nullptr);

			// type
			param_types.push_back(decl_type.first);
			// name
			if (param_annot)
			{
				// FIXME: is this possible?
				auto&& name = decl_type.second.value_or(std::string{});
				param_annot->push_back(name);
			}

		} while (src_.Try(TokenTag::Comma));

		ExpectToken(TokenTag::RParenthesis);
		return GetTypeTable().MakeFunction(ret_type, param_types, va_args)->MakeQualified();
	}

	QualType ParserImpl::ParseArrayDecl(QualType elem_type)
	{
		assert(src_.LastConsumed().Tag == TokenTag::LBracket);

		if (src_.Try(TokenTag::RBracket))
		{
			// size unspecified
			return GetTypeTable().MakeArray(elem_type, 0)->MakeQualified();
		}
		else
		{
			auto expr_sz = ParseAssignmentExpr();
			ExpectToken(TokenTag::RBracket);

			if (expr_sz->IsConstExpr())
			{
				// FIXME: correct size
				return GetTypeTable().MakeArray(elem_type, 0)->MakeQualified();
			}
			else
			{
				throw 0;
			}
		}
	}

	QualType ParserImpl::ParseTypeName()
	{
		auto base_type = ParseDeclSpec(nullptr, nullptr, nullptr, nullptr);
		auto new_type = ParseDeclarator(base_type, nullptr);

		if (new_type.second)
		{
			ReportError(src_.Peek().Location, "name in an abstract declarator");
		}

		return new_type.first;
	}




}