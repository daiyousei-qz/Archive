#include "SourceLexer.h"
#include "Preprocessor.h"
#include "Parser.h"
#include <vector>

int main()
{
	using namespace std;
	using namespace lolita;

	auto file = SourceFile::Open("test.c", "C:\\Users\\Edward Cheng\\Desktop\\test.c");
	auto lex_result = LexSourceFile(file.get());

	auto src = SourceManager{};
	auto diag = DiagonisticClient{};
	InitTranslation(src, diag);

	auto cpp = Preprocessor{ };
	auto cpp_result = cpp.Preprocess(lex_result);

	// auto parse_tree = ParseTranslationUnit(cpp_result);

	for (const auto& tok : cpp_result)
	{
		auto filename = std::string{ tok.Location.File->Name() };
		printf("tok@[line %u; column %u; file %s]: %s\n", tok.Location.Line, tok.Location.Column, filename.c_str(), tok.Content.c_str());
	}

	FinalizeTranslation();

	system("pause");
	return 0;
}