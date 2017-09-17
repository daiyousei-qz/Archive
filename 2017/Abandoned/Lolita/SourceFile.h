#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace lolita
{
	class SourceFile
	{
	private:
		// name of the file
		std::string name_;

		// url to locate the file
		std::string url_;

		// data with standardized newline
		std::string data_;

		// SourceFile instance should be constructed by static function Open
		SourceFile() = default;
	public:
		using Ptr = std::unique_ptr<SourceFile>;

		std::string_view Name() const
		{
			return name_;
		}

		std::string_view Url() const
		{
			return url_;
		}

		size_t Size() const
		{
			return data_.size();
		}

		std::string_view Data() const
		{
			return data_;
		}

		// Factory Function
		//

		static SourceFile::Ptr Open(const std::string& name, const std::string& url);
	};
}