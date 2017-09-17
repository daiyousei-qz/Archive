#pragma once
#include "Token.h"
#include "SourceManager.h"
#include "DiagonisticClient.h"
#include "Type.h"

namespace lolita
{
	// Control
	//
	bool TestTranslationContext();
	void InitTranslation(SourceManager& src, DiagonisticClient& diag);
	void FinalizeTranslation();
	
	void AbortTranslation();
	
	// Error Helper
	//
	void ReportError(const SourceLocation& loc, const std::string& msg);

	void ReportInvalidChar(const Token& tok);
	void ReportUnexpectedNewline(const Token& tok);
	void ReportUnexpectedToken(const Token& tok);
	void ReportUnexpectedToken(const Token& tok, TokenTag expected);

	// Source Helper
	//

	const TokenVec* SearchFile(const std::string& name);

	// Type Helper
	//
	TypeTable& GetTypeTable();
}