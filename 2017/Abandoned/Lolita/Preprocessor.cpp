#include "Preprocessor.h"
#include "Basic.h"
#include "Error.h"
#include <cassert>
#include <vector>
#include <algorithm>
#include <optional>

using namespace std;

namespace lolita
{
	// returns PPDirective::Unknown if not a preprocessing directive
	PPDirective TranslateDirective(const Token& tok)
	{
		static unordered_map<string, PPDirective> directive_map
		{
			{ "if"		, PPDirective::If },
			{ "ifdef"	, PPDirective::IfDef },
			{ "ifndef"	, PPDirective::IfNDef },
			{ "else"	, PPDirective::Else },
			{ "elif"	, PPDirective::ElseIf },
			{ "endif"	, PPDirective::EndIf },
			{ "define"	, PPDirective::Define },
			{ "undef"	, PPDirective::Undef },
			{ "include"	, PPDirective::Include },
			{ "pragma"	, PPDirective::Pragma },
			{ "line"	, PPDirective::Line },
			{ "error"	, PPDirective::Error },
		};

		auto directive_iter = directive_map.find(tok.Content);

		if (directive_iter != directive_map.end())
			return directive_iter->second;
		else
			return PPDirective::Unknown;
	}

	// returns TokenTag::Identifier if not a keyword
	TokenTag TranslateKeyword(const Token& tok)
	{
		static unordered_map<string, TokenTag> keyword_map
		{
			{ "_Alignas"		, TokenTag::Alignas		},
			{ "_Alignof"		, TokenTag::Alignof		},
			{ "_Atomic"			, TokenTag::Atomic		},
			{ "auto"			, TokenTag::Auto		},
			{ "break"			, TokenTag::Break		},
			{ "_Bool"			, TokenTag::Bool		},
			{ "case"			, TokenTag::Case		},
			{ "char"			, TokenTag::Char		},
			{ "_Complex"		, TokenTag::Complex		},
			{ "const"			, TokenTag::Const		},
			{ "continue"		, TokenTag::Continue	},
			{ "default"			, TokenTag::Default		},
			{ "do"				, TokenTag::Do			},
			{ "double"			, TokenTag::Double		},
			{ "else"			, TokenTag::Else		},
			{ "enum"			, TokenTag::Enum		},
			{ "extern"			, TokenTag::Extern		},
			{ "float"			, TokenTag::Float		},
			{ "for"				, TokenTag::For			},
			{ "_Generic"		, TokenTag::Generic		},
			{ "goto"			, TokenTag::Goto		},
			{ "if"				, TokenTag::If			},
			{ "_Imaginary"		, TokenTag::Imaginary	},
			{ "inline"			, TokenTag::Inline		},
			{ "int"				, TokenTag::Int			},
			{ "long"			, TokenTag::Long		},
			{ "_Noreturn"		, TokenTag::Noreturn	},
			{ "register"		, TokenTag::Register	},
			{ "restrict"		, TokenTag::Restrict	},
			{ "return"			, TokenTag::Return		},
			{ "short"			, TokenTag::Short		},
			{ "signed"			, TokenTag::Signed		},
			{ "sizeof"			, TokenTag::Sizeof		},
			{ "static"			, TokenTag::Static		},
			{ "_Static_assert"	, TokenTag::StaticAssert },
			{ "struct"			, TokenTag::Struct		},
			{ "switch"			, TokenTag::Switch		},
			{ "_Thread_local"	, TokenTag::ThreadLocal },
			{ "typedef"			, TokenTag::Typedef		},
			{ "union"			, TokenTag::Union		},
			{ "unsigned"		, TokenTag::Unsigned	},
			{ "void"			, TokenTag::Void		},
			{ "volatile"		, TokenTag::Volatile	},
			{ "while"			, TokenTag::While		},
		};

		auto kwd_iter = keyword_map.find(tok.Content);

		if (kwd_iter != keyword_map.end())
			return kwd_iter->second;
		else
			return TokenTag::Identifier;
	}

	void Preprocessor::PreprocessInternal(const TokenVec& input)
	{
		auto src = TokenSource{ input };
		while (!src.Exhausted())
		{
			const auto& tok = src.Peek();
			switch (tok.Tag)
			{
			case TokenTag::HeaderName:
			case TokenTag::BadHeaderName:
			case TokenTag::Placemarker:
			case TokenTag::EndOfFile:
				// these tags are unexpected
				assert(false);
				break;
			case TokenTag::Sharp:
				if (tok.StartOfLine)
				{
					src.Consume();
					ExecutePP(src);

					break;
				}

				// [[fallthrough]]
			case TokenTag::SharpSharp:
			case TokenTag::InvalidChar:
				// report an error, and omit the token
				ReportInvalidChar(tok);
				src.Consume();
				break;

			case TokenTag::Identifier:
				if (PPTryExpandMacro(src))
					break;

				// [[fallthrough]]
			default:
				FinalizeToken(tok);
				src.Consume();
				break;
			}
		}
	}

	void Preprocessor::FinalizeToken(const Token& tok)
	{
		Token result = tok;

		// refine location track
		result.Location.Line += line_offset_;
		// refine tag if it is a keyword
		if (tok.Tag == TokenTag::Identifier)
			result.Tag = TranslateKeyword(tok);

		buffer_.emplace_back(move(result));
	}

	void Preprocessor::ExecutePP(TokenSource& src)
	{
		// assume # consumed

		// detect null directive
		if (src.StartOfLine())
			return;

		// backup location
		auto loc = src.Peek().Location;

		// parse directive
		auto directive = TranslateDirective(src.Consume());

		// redirect to preprocessor
		switch (directive)
		{
		case PPDirective::Unknown:
			ReportError(loc, "unrecognized preprocessing directive");
			src.OmitCurrentLine();
			break;
		case PPDirective::If:
			ExecutePPIf(src, loc);
			break;
		case PPDirective::IfDef:
			ExecutePPIfDef(src, loc, true);
			break;
		case PPDirective::IfNDef:
			ExecutePPIfDef(src, loc, false);
			break;
		case PPDirective::Else:
			ExecutePPElse(src, loc);
			break;
		case PPDirective::ElseIf:
			ExecutePPElif(src, loc);
			break;
		case PPDirective::EndIf:
			ExecutePPEndIf(src, loc);
			break;
		case PPDirective::Define:
			ExecutePPDefine(src, loc);
			break;
		case PPDirective::Undef:
			ExecutePPUndef(src, loc);
			break;
		case PPDirective::Include:
			ExecutePPInclude(src, loc);
			break;
		case PPDirective::Pragma:
			ExecutePPPragma(src, loc);
			break;
		case PPDirective::Line:
			ExecutePPLine(src, loc);
			break;
		case PPDirective::Error:
			ExecutePPError(src, loc);
			break;
		}

		// omit unused tokens in current line
		if (!src.StartOfLine())
		{
			ReportError(src.Peek().Location, "additional tokens in preprocessing directive");
			src.OmitCurrentLine();
		}
	}

	void Preprocessor::ExecutePPIf(TokenSource& src, SourceLocation loc)
	{
		ReportError(loc, "#if not supported yet");
		src.OmitCurrentLine();
	}
	void Preprocessor::ExecutePPIfDef(TokenSource& src, SourceLocation loc, bool expect_def)
	{
		ReportError(loc, "#ifdef and #ifndef not supported yet");
		src.OmitCurrentLine();
	}
	void Preprocessor::ExecutePPElse(TokenSource& src, SourceLocation loc)
	{
		ReportError(loc, "#else not supported yet");
		src.OmitCurrentLine();
	}
	void Preprocessor::ExecutePPElif(TokenSource& src, SourceLocation loc)
	{
		ReportError(loc, "#elif not supported yet");
		src.OmitCurrentLine();
	}
	void Preprocessor::ExecutePPEndIf(TokenSource& src, SourceLocation loc)
	{
		ReportError(loc, "#endif not supported yet");
		src.OmitCurrentLine();
	}
	void Preprocessor::ExecutePPDefine(TokenSource& src, SourceLocation loc)
	{
		// make a copy of the line for ease of parsing
		auto seq = PPExtractLine(src);
		auto src_cp = TokenSource{ seq };

		// parse macro name
		if (!PPExpectTag(src_cp, TokenTag::Identifier))
			return;

		auto macro_name = src_cp.LastConsumed().Content;
		auto func_like = false;
		auto va_args = false;
		auto params = StringVec{};
		auto rp_list = TokenSeq{};

		if (!src_cp.StartOfLine())
		{
			// then the macro is non-empty

			// load parameters if any
			// NOTE short-circuit here
			if (src_cp.Try(TokenTag::LParenthesis) 
				&& !src_cp.Try(TokenTag::RParenthesis) 
				&& (func_like = true))
			{
				bool expect_id = true; // if expect an identifier or ...
				bool expect_exit = false; // if expect a ')'
				while (true)
				{
					if (src_cp.Exhausted())
					{
						ReportUnexpectedNewline(src_cp.Peek());
						return;
					}

					if (expect_id)
					{
						if (src_cp.Try(TokenTag::Identifier))
						{
							params.emplace_back(src_cp.LastConsumed().Content);
						}
						else if (src_cp.Try(TokenTag::Ellipsis))
						{
							va_args = true;
							expect_exit = true;
						}
						else
						{
							// generate error message
							PPExpectTag(src_cp, TokenTag::Identifier);
							return;
						}
					}
					else
					{
						if (src_cp.Try(TokenTag::Comma))
						{
							if(expect_exit)
								return;
						}
						else if (src_cp.Try(TokenTag::RParenthesis))
						{
							break;
						}
						else
						{
							// generate error message
							PPExpectTag(src_cp, TokenTag::RParenthesis);
							return;
						}
					}

					expect_id = !expect_id;
				}
			}

			// load replacement list
			while (!src_cp.Exhausted())
			{
				rp_list.emplace_back(src_cp.Consume());
			}
		}

		// finalize parsed information
		// e.g. check if parameters have multiple definition
		// ...

		// register macro
		// FIXME: validate macros with the same name
		macros_[macro_name]
			= MacroDescription{ macro_name, move(params), !func_like, va_args, move(rp_list) };
	}
	void Preprocessor::ExecutePPUndef(TokenSource& src, SourceLocation loc)
	{
		if (PPExpectTag(src, TokenTag::Identifier))
		{
			const auto& last_tok = src.LastConsumed();
			macros_.erase(last_tok.Content);
		}
		else
		{
			PPFinalizeSource(src);
		}
	}
	void Preprocessor::ExecutePPInclude(TokenSource& src, SourceLocation loc)
	{
		if (depth_ > 100)
		{
			ReportError(loc, "inclusion depth is too large");
			AbortTranslation();
		}

		if (src.Try(TokenTag::HeaderName))
		{
			const auto& last_tok = src.LastConsumed();
		
			auto toks_to_include = SearchFile(last_tok.Content);
			if (toks_to_include)
			{
				// save context
				auto line_offset = line_offset_;
				depth_ += 1;

				// preprocess included file
				PreprocessInternal(*toks_to_include);

				// restore context
				line_offset_ = line_offset;
				depth_ -= 1;

				return;
			}
			else
			{
				ReportError(last_tok.Location, "cannot lex file ...");
			}
		}
		else
		{
			ReportError(loc, "macro name in #include not supported yet");
		}

		PPFinalizeSource(src);
	}
	void Preprocessor::ExecutePPPragma(TokenSource& src, SourceLocation loc)
	{
		ReportError(loc, "#pragma not supported yet");
		PPFinalizeSource(src);
	}
	void Preprocessor::ExecutePPLine(TokenSource& src, SourceLocation loc)
	{
		if (PPExpectTag(src, TokenTag::IntegerConst))
		{
			// FIXME: switch line
		}
		else
		{
			PPFinalizeSource(src);
		}
	}
	void Preprocessor::ExecutePPError(TokenSource& src, SourceLocation loc)
	{
		if (PPExpectTag(src, TokenTag::StringLiteral))
		{
			const auto& last_tok = src.LastConsumed();
			ReportError(last_tok.Location, last_tok.Content.c_str());
		}
		else
		{
			PPFinalizeSource(src);
		}
	}
}