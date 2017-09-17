#include "SourceFile.h"
#include <fstream>

using namespace std;

namespace lolita
{
	SourceFile::Ptr SourceFile::Open(const std::string& name, const std::string& url)
	{
		fstream fs(url);
		string result;

		// read from file
		constexpr size_t kLoadingBufferSize = 1024;
		char buffer[kLoadingBufferSize];
		while (true)
		{
			fs.read(buffer, sizeof(buffer));

			if (fs.gcount() != 0)
			{
				result.append(buffer, fs.gcount());
			}
			else
			{
				break;
			}
		}

		// to ensure file ends with <newline>
		if (result.empty() || result.back() != '\n')
		{
			result.push_back('\n');
		}

		// check the whole file's been read into memory
		if (!fs.eof())
		{
			return nullptr;
		}

		// construct file object
		auto file = unique_ptr<SourceFile>{ new SourceFile };
		file->name_ = name;
		file->url_ = url;
		file->data_ = move(result);

		return file;
	}
}