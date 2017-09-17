#pragma once
#include "SourceFile.h"
#include <memory>
#include <vector>
#include <unordered_map>

namespace lolita
{
	class SourceManager
	{
	public:
		// NOTE name is raw content of HeaderName
		const SourceFile* Lookup(const std::string& name)
		{
			// fast path, search in cache
			auto cache_iter = cache_.find(name);
			if (cache_iter != cache_.end())
				return cache_iter->second.get();

			// slow path, open the file and lex it
			auto file = OpenFile(name);
			if (file)
			{
				auto result = file.get();
				cache_[name] = std::move(file);

				return result;
			}
			else
			{
				// FIXME: throw?
				return nullptr;
			}
		}

		SourceFile::Ptr OpenFile(const std::string& name)
		{
			assert(name.size() > 2);
			if (name.front() == '"' && name.back() == '"')
			{
				// FIXME:
				auto url = "C:\\Users\\Edward Cheng\\Desktop\\" + name.substr(1, name.size() - 2);
				return SourceFile::Open(name, url);
			}
			else if (name.front() == '<' && name.back() == '>')
			{
				throw;
			}
			else
			{
				return nullptr;
			}
		}

		std::unordered_map<std::string, SourceFile::Ptr> cache_;
	};
}