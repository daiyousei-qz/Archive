#include "TranslationContext.h"
#include "SourceLexer.h"
#include "Error.h"
#include "Type.h"

namespace lolita
{
	struct TranslationContext
	{
		SourceManager& Source;
		DiagonisticClient& Diagonistic;

		std::vector<SourceFile::Ptr> Files;
		std::unordered_map<std::string, std::unique_ptr<TokenVec>>
			LexCache;

		size_t ErrorCount = 0;

		TypeTable Types;
	};

	static TranslationContext*& GetContext()
	{
		static thread_local TranslationContext* ctx;
		return ctx;
	}

	static TranslationContext& EnsureAndGetContext()
	{
		assert(GetContext());
		return *GetContext();
	}

	// Control Impl
	//

	bool TestTranslationContext()
	{
		return GetContext();
	}

	void InitTranslation(SourceManager& src, DiagonisticClient& diag)
	{
		assert(!TestTranslationContext());

		GetContext() = new TranslationContext{ src, diag };
	}
	void FinalizeTranslation()
	{
		assert(TestTranslationContext());

		delete GetContext();
	}

	void AbortTranslation()
	{
		throw 0;
	}

	// Error Impl
	//

	void ReportError(const SourceLocation& loc, const std::string& msg)
	{
		auto& ctx = EnsureAndGetContext();

		ctx.ErrorCount += 1;
		ctx.Diagonistic.ReportError(loc, msg.c_str());
	}

	void ReportInvalidChar(const Token& tok)
	{
		ReportError(tok.Location, "invalid character");
	}
	void ReportUnexpectedNewline(const Token& tok)
	{
		ReportError(tok.Location, "unexpected newline");
	}
	void ReportUnexpectedToken(const Token& tok)
	{
		ReportError(tok.Location, "unexpected token");
	}
	void ReportUnexpectedToken(const Token& tok, TokenTag expected)
	{
		ReportError(tok.Location, "unexpected token");
	}

	// Source Helper
	//

	const TokenVec* SearchFile(const std::string& name)
	{
		auto& ctx = EnsureAndGetContext();

		// fast path, lookup in the cache
		auto iter = ctx.LexCache.find(name);
		if (iter != ctx.LexCache.end())
		{
			return iter->second.get();
		}

		// slow path, open the file and lex
		auto file = ctx.Source.OpenFile(name);
		if (file)
		{
			auto toks = std::make_unique<TokenVec>(LexSourceFile(file.get()));
			auto result = toks.get();

			ctx.Files.push_back(std::move(file));
			ctx.LexCache[name] = std::move(toks);
			return result;
		}
		else
		{
			return nullptr;
		}
	}

	// TypeTable Impl
	//
	TypeTable& GetTypeTable()
	{
		return EnsureAndGetContext().Types;
	}
}