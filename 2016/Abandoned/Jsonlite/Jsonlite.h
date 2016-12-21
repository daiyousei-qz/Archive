#pragma once

#ifndef EDSLIB_BASIC_H
#define EDSLIB_BASIC_H
#include <string>
#include <sstream>

namespace eds
{
#define EDSLIB_DEBUGGING

	//============================================================================
	// primary Exception
	class Exception
	{
	public:
		Exception(const char *msg = "")
			: _message(msg) { }

		const char *Message() const
		{
			return _message;
		}

	private:
		const char *_message;
	};

	//============================================================================
	// assert support

	inline void Assert(bool condition, char *message = "")
	{
#ifdef EDSLIB_DEBUGGING
		if (!condition)
			throw Exception(message);
#endif
	}

	//============================================================================
	// specifier removal facility

	// remove reference
	template <typename T>
	struct RemoveReference
	{
		using Type = T;
	};

	template <typename T>
	struct RemoveReference<T&>
	{
		using Type = T;
	};

	template <typename T>
	struct RemoveReference<T&&>
	{
		using Type = T;
	};

	// remove const
	template <typename T>
	struct RemoveConst
	{
		using Type = T;
	};

	template <typename T>
	struct RemoveConst<const T>
	{
		using Type = T;
	};

	// remove volatile
	template <typename T>
	struct RemoveVolatile
	{
		using Type = T;
	};

	template <typename T>
	struct RemoveVolatile<volatile T>
	{
		using Type = T;
	};

	// remove all specifiers
	template <typename T>
	struct RemoveSpecifier
	{
		using Type = T;
	};

	template <typename T>
	struct RemoveSpecifier<T&>
	{
		using Type = RemoveSpecifier<T>;
	};

	template <typename T>
	struct RemoveSpecifier<T&&>
	{
		using Type = RemoveSpecifier<T>;
	};

	template <typename T>
	struct RemoveSpecifier<const T>
	{
		using Type = RemoveSpecifier<T>;
	};

	template <typename T>
	struct RemoveSpecifier<volatile T>
	{
		using Type = RemoveSpecifier<T>;
	};

	//============================================================================
	// reference transformation facility
	template <typename T>
	inline constexpr typename RemoveReference<T>::Type &&Move(T &&obj)
	{
		return static_cast<typename RemoveReference<T>::Type&&>(obj);
	}

	template<typename T>
	inline constexpr T &&Forward(typename RemoveReference<T>::Type &obj)
	{
		return static_cast<T&&>(obj);
	}

	template<typename T>
	inline constexpr T &&Forward(typename RemoveReference<T>::Type &&obj)
	{
		return static_cast<T&&>(obj);
	}

	//============================================================================
	// basic base class
	class Unconstructible
	{
	public:
		Unconstructible() = delete;
	};

	class Uncopyable
	{
	public:
		Uncopyable() = default;
		Uncopyable(const Uncopyable &) = delete;
		Uncopyable &operator =(const Uncopyable &) = delete;
	};

	//============================================================================
	// utilitiy functions
	template <typename T>
	inline std::string ToString(const T &dat)
	{
		return std::to_string(dat);
	}

	// for floating point types as std::to_string behaves unexpected
	template <>
	inline std::string ToString<double>(const double &dat)
	{
		//if (dat == 0) return "0";
		std::stringstream buf;
		buf << std::fixed << dat;
		std::string s = buf.str();

		// we have to deal with unnecessary sequence of 0s
		int index = s.size() - 1;
		while (s[index] == '0')
		{
			index -= 1;
		}

		if (s[index] == '.')
			s.resize(index);
		else
			s.resize(index + 1);

		return std::move(s);
	}

	template <>
	inline std::string ToString<float>(const float &dat)
	{
		return ToString(static_cast<double>(dat));
	}
}


#endif

#include <cstddef>
#include <string>
#include <stack>
#include <vector>
#include <functional>
#include <sstream>
#include <memory>

// comment this to add serializer support
//#define JSONLITE_SERIALIZATION_DISABLED

namespace jsonlite
{
	namespace internal
	{
		using CharType = char;
		using StringType = std::basic_string<CharType>;
		using stl_ostringstream = std::basic_ostringstream<CharType>;
		using stl_istringstream = std::basic_istringstream<CharType>;

		using ContextMark = std::stringstream::streampos;
		class JsonContext;
	}

	enum class JsonType
	{
		Null,
		Boolean,
		Number,
		String,
		Array,
		Object
	};

#ifndef JSONLITE_SERIALIZATION_DISABLED

	// exported interface classes
	class JsonBuilder;
	class JsonValue;
	class JsonParser;

	// generic serializer for type T
	template <typename T>
	struct JsonSerializer
	{
		// default implementation: to raise compiler errors

		void Serialize(JsonBuilder &builder, const T &dat)
		{
			static_assert(false, "No serializer of the type is available.");
		}

		T Deserialize(const JsonValue &value)
		{
			static_assert(false, "No deserializer of the type is available.");
		}
	};

#endif // !JSONLITE_SERIALIZATION_DISABLED

	//============================================================================
	// JsonBuilder
	class JsonBuilder : public eds::Uncopyable
	{
	public:
		JsonBuilder(internal::stl_ostringstream &writer);
		~JsonBuilder();

#ifndef JSONLITE_SERIALIZATION_DISABLED

		template <typename T>
		void Serialize(const T &dat)
		{
			JsonSerializer<typename eds::RemoveSpecifier<T>::Type>().Serialize(*this, dat);
		}

		template <typename T>
		void SerializeAsField(const internal::StringType &key, const T &dat)
		{
			FeedKey(key);
			Serialize<T>(dat);
		}

#endif // !JSONLITE_SERIALIZATION_DISABLED

		void FeedKey(const std::string &key);

		void FeedNull();
		void FeedBoolean(bool value);
		void FeedDouble(double value);
		void FeedString(const std::string &value);

		void FeedArray(const std::function<void()> &callback);
		void FeedObject(const std::function<void()> &callback);

	private:
		class InternalImpl;
		std::unique_ptr<InternalImpl> _impl;
	};

	//============================================================================
	// JsonValue
	class JsonValue
	{
	public:
		JsonValue(internal::JsonContext &ctx);

		JsonType GetType() const { return _type; }

#ifndef JSONLITE_SERIALIZATION_DISABLED

		template <typename T>
		typename eds::RemoveSpecifier<T>::Type Deserialize() const
		{
			auto serializer = JsonSerializer<typename eds::RemoveSpecifier<T>::Type>();
			return std::move(serializer.Deserialize(*this));
		}

#endif // !JSONLITE_SERIALIZATION_DISABLED

		bool AsBoolean() const;
		double AsDouble() const;
		internal::StringType AsString() const;

		void AsArray(const std::function<void(const JsonValue &)> &callback) const;
		void AsObject(const std::function<void(const internal::StringType &, const JsonValue &)> &callback) const;

		internal::StringType ToString() const;

	private:
		JsonType _type;
		internal::JsonContext &_ctx;
		internal::ContextMark _mark;
	};

	//============================================================================
	// JsonParser
	class JsonParser : public eds::Uncopyable
	{
	public:
		JsonParser(internal::stl_istringstream &src);
		~JsonParser();

		JsonValue GetValue();
	private:
		std::unique_ptr<internal::JsonContext> _ctx;
	};

} // namespace jsonlite

#ifndef JSONLITE_SERIALIZATION_DISABLED

namespace jsonlite
{
	//============================================================================
	// default serializers
	template<>
	struct JsonSerializer<bool>
	{
		void Serialize(JsonBuilder &builder, const bool &dat)
		{
			return builder.FeedBoolean(dat);
		}
		bool Deserialize(const JsonValue &value)
		{
			return value.AsBoolean();
		}
	};

	template<>
	struct JsonSerializer<int8_t>
	{
		void Serialize(JsonBuilder &builder, const int8_t &dat)
		{
			return builder.FeedDouble(static_cast<double>(dat));
		}
		int8_t Deserialize(const JsonValue &value)
		{
			return static_cast<int8_t>(value.AsDouble());
		}
	};

	template<>
	struct JsonSerializer<uint8_t>
	{
		void Serialize(JsonBuilder &builder, const uint8_t &dat)
		{
			return builder.FeedDouble(static_cast<double>(dat));
		}
		uint8_t Deserialize(const JsonValue &value)
		{
			return static_cast<uint8_t>(value.AsDouble());
		}
	};

	template<>
	struct JsonSerializer<int16_t>
	{
		void Serialize(JsonBuilder &builder, const int16_t &dat)
		{
			return builder.FeedDouble(static_cast<double>(dat));
		}
		int16_t Deserialize(const JsonValue &value)
		{
			return static_cast<int16_t>(value.AsDouble());
		}
	};

	template<>
	struct JsonSerializer<uint16_t>
	{
		void Serialize(JsonBuilder &builder, const uint16_t &dat)
		{
			return builder.FeedDouble(static_cast<double>(dat));
		}
		uint16_t Deserialize(const JsonValue &value)
		{
			return static_cast<uint16_t>(value.AsDouble());
		}
	};

	template<>
	struct JsonSerializer<int32_t>
	{
		void Serialize(JsonBuilder &builder, const int32_t &dat)
		{
			return builder.FeedDouble(static_cast<double>(dat));
		}
		int32_t Deserialize(const JsonValue &value)
		{
			return static_cast<int32_t>(value.AsDouble());
		}
	};

	template<>
	struct JsonSerializer<uint32_t>
	{
		void Serialize(JsonBuilder &builder, const uint32_t &dat)
		{
			return builder.FeedDouble(static_cast<double>(dat));
		}
		uint32_t Deserialize(const JsonValue &value)
		{
			return static_cast<uint32_t>(value.AsDouble());
		}
	};

	template<>
	struct JsonSerializer<float>
	{
		void Serialize(JsonBuilder &builder, const float &dat)
		{
			return builder.FeedDouble(static_cast<double>(dat));
		}
		float Deserialize(const JsonValue &value)
		{
			return static_cast<float>(value.AsDouble());
		}
	};

	template<>
	struct JsonSerializer<double>
	{
		void Serialize(JsonBuilder &builder, const double &dat)
		{
			return builder.FeedDouble(dat);
		}
		double Deserialize(const JsonValue &value)
		{
			return value.AsDouble();
		}
	};

	template<>
	struct JsonSerializer<internal::StringType>
	{
		void Serialize(JsonBuilder &builder, const internal::StringType &dat)
		{
			return builder.FeedString(dat);
		}
		internal::StringType Deserialize(const JsonValue &value)
		{
			return std::move(value.AsString());
		}
	};

	// generic array based on std::vector
	template <typename T>
	struct JsonSerializer<std::vector<T>>
	{
		void Serialize(JsonBuilder &builder, const std::vector<T> &dat)
		{
			builder.FeedArray([&]() {
				JsonSerializer<T> elementSerializer;
				for (const T &x : dat)
				{
					elementSerializer.Serialize(builder, x);
				}
			});
		}
		std::vector<T> Deserialize(const JsonValue &value)
		{
			std::vector<T> tmp;
			value.AsArray([&tmp](const JsonValue &value) {
				tmp.push_back(value.Deserialize<T>());
			});

			return move(tmp);
		}
	};
} // namespace jsonlite

#endif // !JSONLITE_SERIALIZATION_DISABLED