/*///===========================================================
* Name: Jsonlite.hpp
* Author: Edward Cheng
* Decs: Jsonlite is a powerful json library in modern C++
/*///===========================================================

#include "Jsonlite.h"
#include <string>
#include <sstream>
#include <stack>
#include <vector>
#include <functional>

using namespace eds;

namespace jsonlite
{
	namespace internal
	{
		constexpr CharType JSON_BEGIN_OBJECT = '{';
		constexpr CharType JSON_END_OBJECT = '}';
		constexpr CharType JSON_BEGIN_ARRAY = '[';
		constexpr CharType JSON_END_ARRAY = ']';
		constexpr CharType JSON_BEGIN_STRING = '\"';
		constexpr CharType JSON_END_STRING = '\"';
		constexpr CharType JSON_ESCAPE_CHARACTER = '\\';
		constexpr CharType JSON_ELEMENT_SAPERATOR = ',';
		constexpr CharType JSON_PAIR_SAPERATOR = ':';

		constexpr CharType *JSON_LITERAL_NULL = "null";
		constexpr CharType *JSON_LITERAL_TRUE = "true";
		constexpr CharType *JSON_LITERAL_FALSE = "false";

		// a JsonContext is a wrapper of input text stream
		class JsonContext : public eds::Uncopyable
		{
		public:
			JsonContext(std::istringstream &src) : _initialPos(src.tellg()), _source(src) { }

			ContextMark GetInitialMark() const
			{
				return _initialPos;
			}
			ContextMark Record() const
			{
				return _source.tellg();
			}
			void SwitchTo(internal::ContextMark mark)
			{
				_source.seekg(mark);
			}

			// peek type from the current position(just a guess of type, but not verified)
			JsonType PeekType()
			{
				char ch = _PeekNextNonWhitespace();

				switch (ch)
				{
					// 'n'
				case JSON_LITERAL_NULL[0]:
					return JsonType::Null;
					// 't' or 'f'
				case JSON_LITERAL_TRUE[0]:
				case JSON_LITERAL_FALSE[0]:
					return JsonType::Boolean;
					// '\"'
				case JSON_BEGIN_STRING:
					return JsonType::String;
					// '['
				case JSON_BEGIN_ARRAY:
					return JsonType::Array;
					// '{'
				case JSON_BEGIN_OBJECT:
					return JsonType::Object;
				default:
					// then it should be a number
					if (isdigit(ch) || ch == '-')
					{
						return JsonType::Number;
					}
					else
					{
						throw Exception("Invalid character to start a json entity.");
					}
				}
			}

			// these functions only advance index in the context, which is usually faster then parsing
			void AdvanceNull()
			{
				_AssertLiteral(JSON_LITERAL_NULL);

			}
			void AdvanceBoolean()
			{
				ParseBoolean();
			}
			void AdvanceNumber()
			{
				ParseNumber();
			}
			void AdvanceString()
			{
				_AssertNextNonWhitespace(internal::JSON_BEGIN_STRING);

				for (CharType ch = _LoadNext(); ch != internal::JSON_END_STRING; ch = _LoadNext())
				{
					if (ch == internal::JSON_ESCAPE_CHARACTER)
					{
						_LoadEscapedChar();
					}
				}
			}

			void AdvanceObject()
			{
				_AssertNextNonWhitespace(internal::JSON_BEGIN_OBJECT);

				// if not an empty object
				if (!_CheckNextNonWhitespace(internal::JSON_END_OBJECT))
				{
					while (1)
					{
						// skip name
						AdvanceString();
						// assert saperator
						_AssertNextNonWhitespace(internal::JSON_PAIR_SAPERATOR);
						// skip value
						_AdvanceValue();


						if (_CheckNextNonWhitespace(internal::JSON_END_OBJECT))
						{
							break;
						}
						else
						{
							_AssertNextNonWhitespace(internal::JSON_ELEMENT_SAPERATOR);
						}
					}
				}

				_AssertNextNonWhitespace(internal::JSON_END_OBJECT);
			}
			void AdvanceArray()
			{
				_AssertNextNonWhitespace(internal::JSON_BEGIN_ARRAY);
				// if not an empty array
				if (!_CheckNextNonWhitespace(internal::JSON_END_ARRAY))
				{
					while (1)
					{
						// skip value
						_AdvanceValue();

						if (_CheckNextNonWhitespace(internal::JSON_END_ARRAY))
						{
							break;
						}
						else
						{
							_AssertNextNonWhitespace(internal::JSON_ELEMENT_SAPERATOR);
						}
					}
				}

				_AssertNextNonWhitespace(internal::JSON_END_ARRAY);
			}

			// these functions parse value from the given context
			bool ParseBoolean()
			{
				if (_CheckNextNonWhitespace(JSON_LITERAL_TRUE[0]))
				{
					_AssertLiteral(JSON_LITERAL_TRUE);
					return true;
				}
				else if (_CheckNextNonWhitespace(JSON_LITERAL_FALSE[0]))
				{
					_AssertLiteral(JSON_LITERAL_FALSE);
					return false;
				}
				else
				{
					throw Exception("Not a valid boolean value.");
				}
			}

			// ParseNumber can and should be simplified
			double ParseNumber()
			{
				// loads number into buffer first
				// and then return the converted value

				CharType buf[128] = {};
				CharType ch = _LoadNextNonWhitespace();
				// starting character of a number should be either - or a digit
				if (!(isdigit(ch) || ch == '-')) throw Exception("Not a number value.");
				buf[0] = ch;

				bool fracExpected = true;
				bool eExpected = true;
				int index = 1;
				while (1)
				{
					ch = _PeekNext();
					if (isdigit(ch))
					{
						// a digit is always welcomed
						buf[index++] = _LoadNext();
					}
					else if (ch == '.' && fracExpected)
					{
						// only take the first fraction point
						buf[index++] = _LoadNext();
						fracExpected = false;
					}
					else if (tolower(ch) == 'e' && eExpected)
					{
						// only take the first e/E indicator
						buf[index++] = _LoadNext();
						eExpected = false;
						fracExpected = false; // no fraction point after e/E indicator

											  // check possible sign character
						CharType chNext = _PeekNext();
						if (chNext == '+' || chNext == '-')
							buf[index++] = _LoadNext();
					}
					else
					{
						// any other character is not included in a number
						break;
					}
				}

				return atof(buf);
			}
			internal::StringType ParseString()
			{
				_AssertNextNonWhitespace(internal::JSON_BEGIN_STRING);

				// loop until JSON_END_STRING is read
				internal::StringType s;
				for (CharType ch = _LoadNext(); ch != internal::JSON_END_STRING; ch = _LoadNext())
				{
					if (ch == internal::JSON_ESCAPE_CHARACTER)
					{
						s.push_back(_LoadEscapedChar());
					}
					else
					{
						s.push_back(ch);
					}
				}

				return Move(s);
			}

			void ParseObject(std::function<void(const internal::StringType &)> callback)
			{
				_AssertNextNonWhitespace(internal::JSON_BEGIN_OBJECT);

				// if not an empty object
				if (!_CheckNextNonWhitespace(internal::JSON_END_OBJECT))
				{
					while (1)
					{
						internal::StringType name = ParseString();
						_AssertNextNonWhitespace(internal::JSON_PAIR_SAPERATOR);

						// record mark to see if position is advanced in callback
						auto mark = Record();
						callback(name);
						// if mark is unchanged( indicating user did't parse the data in the callback)
						// advance and skip the value in the context
						if (mark == Record())
						{
							_AdvanceValue();
						}

						if (_CheckNextNonWhitespace(internal::JSON_END_OBJECT))
						{
							break;
						}
						else
						{
							_AssertNextNonWhitespace(internal::JSON_ELEMENT_SAPERATOR);
						}
					}
				}

				_AssertNextNonWhitespace(internal::JSON_END_OBJECT);
			}
			void ParseArray(std::function<void()> callback)
			{
				_AssertNextNonWhitespace(internal::JSON_BEGIN_ARRAY);
				// if not an empty array
				if (!_CheckNextNonWhitespace(internal::JSON_END_ARRAY))
				{
					while (1)
					{
						auto mark = Record();
						callback();
						// if mark is unchanged( indicating user did't parse the data in the callback)
						// advance and skip the value in the context
						if (mark == Record())
						{
							_AdvanceValue();
						}

						if (_CheckNextNonWhitespace(internal::JSON_END_ARRAY))
						{
							break;
						}
						else
						{
							_AssertNextNonWhitespace(internal::JSON_ELEMENT_SAPERATOR);
						}
					}
				}

				_AssertNextNonWhitespace(internal::JSON_END_ARRAY);
			}

		private:
			internal::CharType _LoadNext()
			{
				return _source.get();
			}
			internal::CharType _PeekNext()
			{
				return _source.peek();
			}
			internal::CharType _LoadNextNonWhitespace()
			{
				internal::CharType tmp;
				while (isspace(tmp = _source.get()))
				{
					if (_source.eof()) throw Exception("Unexpected EOF encountered");
				}

				return tmp;
			}
			internal::CharType _PeekNextNonWhitespace()
			{
				internal::CharType tmp = _LoadNextNonWhitespace();

				_source.unget();
				return tmp;
			}

			bool _CheckNextNonWhitespace(internal::CharType ch)
			{
				return _PeekNextNonWhitespace() == ch;
			}
			void _AssertNextNonWhitespace(internal::CharType ch)
			{
				internal::CharType next = _LoadNextNonWhitespace();

				Assert(next == ch, "Unexpected character encountered");
			}
			void _AssertLiteral(const CharType *p)
			{
				while (*p != '\0')
				{
					_AssertNextNonWhitespace(*p);
					++p;
				}
			}

			void _AdvanceValue()
			{
				JsonType valueType = PeekType();
				switch (valueType)
				{
				case JsonType::Null:
					AdvanceNull();
					break;
				case JsonType::Boolean:
					AdvanceBoolean();
					break;
				case JsonType::Number:
					AdvanceNumber();
					break;
				case JsonType::String:
					AdvanceString();
					break;
				case JsonType::Array:
					AdvanceArray();
					break;
				case JsonType::Object:
					AdvanceObject();
					break;
				default:
					// this exception will never be thrown as return value of PeekType() is controlled
					throw Exception("Unknown JsonType enumeration.");
				}
			}
			internal::CharType _LoadEscapedChar()
			{
				internal::CharType ch = _LoadNext();

				switch (ch)
				{
				case 'b':
					return '\b';
				case 'f':
					return '\f';
				case 'r':
					return '\r';
				case 'n':
					return '\n';
				case 't':
					return '\t';
				case '\"':
					return '\"';
				case '/':
					return '/';
				case '\\':
					return '\\';

					// unexpected character to escape
				default:
					throw Exception("Unexpected characters to escape.");
				}
			}

			internal::ContextMark _initialPos;
			internal::stl_istringstream &_source;
		};
	}

	//============================================================================
	// JsonBuilder
	class JsonBuilder::InternalImpl
	{
	public:
		enum BuilderState
		{
			InArray,
			InObject,
			ValueWaited,
		};

		InternalImpl(internal::stl_ostringstream &writer) : _output(writer) { }

		bool CheckState(BuilderState state)
		{
			if (_states.empty())
			{
				return false;
			}
			else if (_states.top() != state)
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		void PushState(BuilderState state)
		{
			_states.push(state);
		}
		void EnterCompoundState(BuilderState state, internal::CharType starter)
		{
			_states.push(state);
			_sapNeeded = false;

			_output.put(starter);
		}
		void ExitCompoundState(BuilderState state, internal::CharType ending)
		{
			if (_states.top() != state)
				throw Exception("Trying to close a unmatched state.");

			_states.pop();
			_sapNeeded = true;
			_output.put(ending);
		}

		// with escape
		void WriteRawString(const internal::StringType &str)
		{
			_output << str;
		}
		void WriteEscapedString(const internal::StringType &str)
		{
			char buf[128 + 2];
			size_t bufIndex = 0;

			const char *p = str.c_str();
			for (size_t i = 0; i < str.size(); ++i)
			{
				if (bufIndex > 128)
				{
					_output.write(buf, bufIndex);
					bufIndex = 0;
				}

				// check if escape is needed
				char toEscape = 0;
				switch (p[i])
				{
				case '\"':
					toEscape = '\"';
					break;
				case '\\':
					toEscape = '\\';
					break;
				case '/':
					toEscape = '/';
					break;
				case '\n':
					toEscape = 'n';
					break;
				case '\r':
					toEscape = 'r';
					break;
				case '\t':
					toEscape = 't';
					break;
				case '\f':
					toEscape = 'f';
					break;
				case '\b':
					toEscape = 'b';
					break;
				default:
					break;
				}

				if (toEscape != 0)
				{
					buf[bufIndex++] = '\\';
					buf[bufIndex++] = toEscape;
				}
				else
				{
					buf[bufIndex++] = p[i];
				}
			}

			// write data remaining in the buffer
			if (bufIndex != 0)
			{
				_output.write(buf, bufIndex);
			}
		}
		void PrepareFeeding()
		{
			if (_states.empty()) return;

			if (_states.top() == BuilderState::InArray | _states.top() == BuilderState::InObject)
			{
				if (_sapNeeded)
				{
					_output.put(internal::JSON_ELEMENT_SAPERATOR);
				}
				else
				{
					_sapNeeded = true;
				}
			}
			else if (_states.top() == BuilderState::ValueWaited)
			{
				_output.put(internal::JSON_PAIR_SAPERATOR);
				_states.pop();
			}
			else
			{
				throw eds::Exception();
			}
		}
		void FeedStringInternal(const internal::StringType &str)
		{
			_output.put(internal::JSON_BEGIN_STRING);
			WriteEscapedString(str);
			_output.put(internal::JSON_END_STRING);
		}

		void OpenArray()
		{
			PrepareFeeding();
			EnterCompoundState(BuilderState::InArray, internal::JSON_BEGIN_ARRAY);
		}
		void CloseArray()
		{
			ExitCompoundState(BuilderState::InArray, internal::JSON_END_ARRAY);
		}
		void OpenObject()
		{
			PrepareFeeding();
			EnterCompoundState(BuilderState::InObject, internal::JSON_BEGIN_OBJECT);
		}
		void CloseObject()
		{
			ExitCompoundState(BuilderState::InObject, internal::JSON_END_OBJECT);
		}


	private:
		bool _sapNeeded = false;
		std::stack<BuilderState> _states;
		jsonlite::internal::stl_ostringstream &_output;
	};

	JsonBuilder::JsonBuilder(internal::stl_ostringstream &writer) : _impl(std::make_unique<InternalImpl>(writer)) { }
	JsonBuilder::~JsonBuilder() { }

	void JsonBuilder::FeedKey(const std::string &key)
	{
		if (!_impl->CheckState(InternalImpl::BuilderState::InObject))
			throw eds::Exception("A field key is not expected.");

		_impl->PrepareFeeding();
		_impl->FeedStringInternal(key);
		_impl->PushState(InternalImpl::BuilderState::ValueWaited);
	}

	void JsonBuilder::FeedNull()
	{
		_impl->PrepareFeeding();
		_impl->WriteRawString(internal::JSON_LITERAL_NULL);
	}
	void JsonBuilder::FeedBoolean(bool value)
	{
		_impl->PrepareFeeding();
		_impl->WriteRawString(value ? internal::JSON_LITERAL_TRUE : internal::JSON_LITERAL_FALSE);
	}
	void JsonBuilder::FeedDouble(double value)
	{
		_impl->PrepareFeeding();
		_impl->WriteRawString(eds::ToString(value));
	}
	void JsonBuilder::FeedString(const std::string &value)
	{
		_impl->PrepareFeeding();
		_impl->FeedStringInternal(value);
	}

	void JsonBuilder::FeedArray(const std::function<void()> &callback)
	{
		_impl->OpenArray();
		callback();
		_impl->CloseArray();
	}
	void JsonBuilder::FeedObject(const std::function<void()> &callback)
	{
		_impl->OpenObject();
		callback();
		_impl->CloseObject();
	}
	//*
	//============================================================================
	// JsonValue
	JsonValue::JsonValue(internal::JsonContext &ctx)
		: _ctx(ctx), _type(ctx.PeekType()), _mark(ctx.Record()) { }

	bool JsonValue::AsBoolean() const
	{
		_ctx.SwitchTo(_mark);
		return _ctx.ParseBoolean();
	}
	double JsonValue::AsDouble() const
	{
		_ctx.SwitchTo(_mark);
		return _ctx.ParseNumber();
	}
	internal::StringType JsonValue::AsString() const
	{
		_ctx.SwitchTo(_mark);
		return Move(_ctx.ParseString());
	}

	// WARNING!!!
	// ways of implementation of these two function should be modified to be more intuitive
	void JsonValue::AsArray(const std::function<void(const JsonValue &)> &callback) const
	{
		_ctx.ParseArray([&]() {
			callback(JsonValue(_ctx));
		});
	}
	void JsonValue::AsObject(const std::function<void(const internal::StringType &, const JsonValue &)> &callback) const
	{
		_ctx.ParseObject([&](const internal::StringType &key) {
			callback(key, JsonValue(_ctx));
		});
	}

	internal::StringType JsonValue::ToString() const
	{
		switch (GetType())
		{
		case JsonType::Null:
			return "null";
		case JsonType::Boolean:
			return  AsBoolean() ? "true" : "false";
		case JsonType::Number:
			return Move(std::to_string(AsDouble()));
		case JsonType::String:
			return Move(AsString());
		case JsonType::Array:
			return "JsonArray";
		case JsonType::Object:
			return "JsonObject";
		default:
			// this exception will never be thrown.
			throw Exception("Unknown type of JsonValue.");
		}
	}

	//============================================================================
	// JsonParser

	JsonParser::JsonParser(internal::stl_istringstream &src)
		: _ctx(std::make_unique<internal::JsonContext>(src)) { }
	JsonParser::~JsonParser() { }

	JsonValue JsonParser::GetValue()
	{
		_ctx->SwitchTo(_ctx->GetInitialMark());
		return JsonValue(*_ctx);
	}
	//*/
}