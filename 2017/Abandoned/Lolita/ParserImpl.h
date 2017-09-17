#pragma once
#include "Basic.h"
#include "TranslationContext.h"
#include "Token.h"
#include "AstBuilder.h"
#include "Decl.h"
#include "Error.h"
#include <memory>
#include <string>
#include <optional>

namespace lolita
{
	class TypeSpec
	{
		
	};

	class TypeSpecContext
	{
	public:
		TypeSpecContext()
		{
			Initialize();
		}

		void Initialize()
		{
			state_ = MajorState::None;

			float_state_ = FloatState::Float;

			int_state_.ExplicitInt = false;
			int_state_.Signedness = IntSignedness::None;
			int_state_.Width = IntWidth::None;
		}

		bool IsEmpty()
		{
			return state_ == MajorState::None;
		}

		bool IsBuiltinType()
		{
			return state_ != MajorState::None
				&& state_ != MajorState::Custom;
		}

		void SetCustom()
		{
			if (state_ != MajorState::None)
				throw 0;

			state_ = MajorState::Custom;
		}

		void Feed(TokenTag tag)
		{
			if (state_ == MajorState::None)
				FeedWhenEmpty(tag);
			else
				FeedWhenNotEmpty(tag);
		}

		BuiltinType Export()
		{
			assert(IsBuiltinType());
			switch (state_)
			{
			case MajorState::Void:
				return BuiltinType::Void;
			case MajorState::Int:
			{
				auto is_signed = int_state_.Signedness != IntSignedness::Unsigned;
				auto width =
					int_state_.Width != IntWidth::None
					? int_state_.Width
					: IntWidth::Long;

				if (width == IntWidth::Char)
				{
					return is_signed ? BuiltinType::Int8 : BuiltinType::UInt8;
				}
				else if (width == IntWidth::Short)
				{
					return is_signed ? BuiltinType::Int16 : BuiltinType::UInt16;
				}
				else if (width == IntWidth::Long)
				{
					return is_signed ? BuiltinType::Int32 : BuiltinType::UInt32;
				}
				else if (width == IntWidth::LongLong)
				{
					return is_signed ? BuiltinType::Int64 : BuiltinType::UInt64;
				}
			}
			case MajorState::Float:
				if (float_state_ == FloatState::Float)
					return BuiltinType::Float32;
				else
					return BuiltinType::Float64;
			}
		}

	private:
		void FeedWhenEmpty(TokenTag tag)
		{
			assert(state_ == MajorState::None);
			switch (tag)
			{
			case TokenTag::Void:
				state_ = MajorState::Void;
				break;
			case TokenTag::Int:
				state_ = MajorState::Int;
				int_state_.ExplicitInt = true;
				break;
			case TokenTag::Char:
				state_ = MajorState::Int;
				int_state_.ExplicitInt = true;
				int_state_.Width = IntWidth::Char;
				break;
			case TokenTag::Short:
				state_ = MajorState::Int;
				int_state_.Width = IntWidth::Short;
				break;
			case TokenTag::Long:
				state_ = MajorState::Int;
				int_state_.Width = IntWidth::Long;
				break;
			case TokenTag::Signed:
				state_ = MajorState::Int;
				int_state_.Signedness = IntSignedness::Signed;
				break;
			case TokenTag::Unsigned:
				state_ = MajorState::Int;
				int_state_.Signedness = IntSignedness::Unsigned;
				break;
			case TokenTag::Float:
				state_ = MajorState::Float;
				float_state_ = FloatState::Float;
				break;
			case TokenTag::Double:
				state_ = MajorState::Float;
				float_state_ = FloatState::Double;
				break;
			default:
				throw 0;
			}
		}

		void FeedWhenNotEmpty(TokenTag tag)
		{
			switch (tag)
			{
			case TokenTag::Int:
				if (state_ == MajorState::Int
					&& !int_state_.ExplicitInt)
				{
					int_state_.ExplicitInt = true;
					return;
				}
				break;
			case TokenTag::Char:
				if (state_ == MajorState::Int
					&& !int_state_.ExplicitInt
					&& int_state_.Width == IntWidth::None)
				{
					int_state_.ExplicitInt = true;
					int_state_.Width = IntWidth::Char;
					return;
				}
			case TokenTag::Signed:
			case TokenTag::Unsigned:
				if (state_ == MajorState::Int
					&& int_state_.Signedness == IntSignedness::None)
				{
					int_state_.Signedness =
						tag == TokenTag::Signed
						? IntSignedness::Signed
						: IntSignedness::Unsigned;
					return;
				}
				break;
			case TokenTag::Short:
				if (state_ == MajorState::Int
					&& int_state_.Width == IntWidth::None)
				{
					int_state_.Width = IntWidth::Short;
					return;
				}
				break;
			case TokenTag::Long:
				if (state_ == MajorState::Int)
				{
					if (int_state_.Width == IntWidth::None)
					{
						int_state_.Width = IntWidth::Long;
						return;
					}
					else if (int_state_.Width == IntWidth::Long)
					{
						int_state_.Width = IntWidth::LongLong;
						return;
					}
				}
				else if (state_ == MajorState::Float)
				{
					if (float_state_ == FloatState::Double)
					{
						float_state_ = FloatState::LongDouble;
						return;
					}
				}
				break;
			case TokenTag::Double:
				if (state_ == MajorState::Int
					&& !int_state_.ExplicitInt
					&& int_state_.Signedness == IntSignedness::None
					&& int_state_.Width == IntWidth::Long)
				{
					state_ = MajorState::Float;
					float_state_ = FloatState::LongDouble;
					return;
				}
				break;
			}

			throw 0;
		}

		enum class MajorState
		{
			None, Void, Int, Float, Custom,
		};

		enum class IntSignedness
		{
			None, Signed, Unsigned,
		};

		enum class IntWidth
		{
			None, Char, Short, Long, LongLong
		};

		enum class FloatState
		{
			Float, Double, LongDouble
		};

		struct IntState
		{
			bool ExplicitInt;

			IntSignedness Signedness;
			IntWidth Width;
		};

		MajorState state_;
		FloatState float_state_;
		IntState int_state_;
	};

	using DeclaratorInfo = std::pair<
		QualType,
		std::optional<std::string>
	>;

	class ParserImpl
	{
	public:
		ParserImpl(TokenSource& src)
			: src_(src) { }

		// Parsing Translation Unit
		//

		DeclBase* ParseStaticAssert();

		DeclBase* ParseDeclaration(FunctionSpecifier* func, StorageSpecifier* storage);
		std::vector<DeclBase*> ParseTranslationUnit();

		// Parsing Type
		//

		bool TryAppendQual(TypeQualifier& quals);
		bool TryAppendFuncSpec(FunctionSpecifier& spec);
		bool TryAppendStorageSpec(StorageSpecifier& spec);
		bool TryAppendAlignas(size_t& align);

		TypeQualifier ParseTypeQualifierList();

		// FIXME: clean up the mess
		QualType ParseDeclSpec(
			FunctionSpecifier* func_spec,
			StorageSpecifier* storage_spec,
			bool* thd_local_spec,
			size_t* align_spec);

		DeclaratorInfo ParseDeclarator(QualType base, StringVec* param_annot);
		QualType ParsePointer(QualType type);
		QualType ParseFuncDecl(QualType ret_type, StringVec* param_annot);
		QualType ParseArrayDecl(QualType elem_type);

		QualType ParseTypeName();

		// Parsing Expr
		//
		ExprBase* ParseGenericSelection();
		ExprBase* ParsePrimaryExpr();
		ExprBase* ParseCompoundLiteral();
		ExprBase* ParsePostfixExpr();
		ExprBase* ParseUnaryExpr();
		ExprBase* ParseCastExpr();
		ExprBase* ParseMultiplicativeExpr();
		ExprBase* ParseAdditiveExpr();
		ExprBase* ParseShiftExpr();
		ExprBase* ParseRelationalExpr();
		ExprBase* ParseEqualityExpr();
		ExprBase* ParseBitwiseAndExpr();
		ExprBase* ParseBitwiseXorExpr();
		ExprBase* ParseBitwiseOrExpr();
		ExprBase* ParseLogicalAndExpr();
		ExprBase* ParseLogicalOrExpr();
		ExprBase* ParseConditionalExpr();
		ExprBase* ParseAssignmentExpr();
		ExprBase* ParseCommaExpr();
		ExprBase* ParseExpr();

		// assuming left associativity
		template <ExprBase* (ParserImpl::*NextParseFunc)(), TokenTag ... Ps>
		ExprBase* ParseNaiveBinaryExpr()
		{
			ExprBase* lhs = (this->*NextParseFunc)();
			while (!src_.Exhausted())
			{
				auto tok = src_.Peek();
				auto ps = initializer_list<TokenTag>{ Ps... };
				auto iter = find(ps.begin(), ps.end(), tok.Tag);
				if (iter != ps.end())
				{
					// consume the token
					src_.Consume();
					// prepare other parts of the expression
					BinaryOp op = *TranslateBinaryOp(*iter);
					ExprBase* rhs = (this->*NextParseFunc)();
					// construct expression
					lhs = arena_.NewBinaryExpr(op, lhs, rhs);
					continue;
				}

				break;
			}

			return lhs;
		}

		// Parsing Stmt
		//
		StmtBase* ParseDeclStmt();
		StmtBase* ParseExprStmt();
		StmtBase* ParseIfStmt();
		StmtBase* ParseSwitchStmt();
		StmtBase* ParseForStmt();
		StmtBase* ParseWhileStmt();
		StmtBase* ParseDoWhileStmt();
		StmtBase* ParseCompoundStmt();

		StmtBase* ParseUnlabelledStmt();
		StmtBase* ParseStmt();
	private:
		// Utils
		//
		bool DetectExpression()
		{
			switch (src_.Peek().Tag)
			{
			case TokenTag::Identifier:
				// FIXME: check declarations
				return true;
			case TokenTag::IntegerConst:
			case TokenTag::FloatConst:
			case TokenTag::CharConst:
			case TokenTag::StringLiteral:
			case TokenTag::Plus:
			case TokenTag::Minus:
			case TokenTag::Asterisk:
			case TokenTag::Ampersand:
			case TokenTag::Exclamation:
			case TokenTag::Tlide:
			case TokenTag::Increment:
			case TokenTag::Decrement:
				return true;
			default:
				return false;
			}
		}
		void ExpectToken(TokenTag tag)
		{
			if (!src_.Try(tag))
			{
				ReportUnexpectedToken(src_.Peek(), tag);
				throw ParsingError{ };
			}
		}

		// ======================================

		TokenSource src_;

		AstBuilder builder_;
		
		// remove fields below
		AstArena arena_;
		Scope* file_scope_;
		Scope* local_scope_;
	};
}