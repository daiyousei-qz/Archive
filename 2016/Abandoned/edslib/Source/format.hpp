/*=================================================================================
 *  Copyright (c) 2016 Edward Cheng
 *
 *  edslib is an open-source library in C++ and licensed under the MIT License. 
 *  Refer to: https://opensource.org/licenses/MIT
 *================================================================================*/

#pragma once
#include "string.hpp"
#include "encoding.hpp"
#include "static_vector.hpp"
#include <stdexcept>
#include <functional>
#include <string>
#include <sstream>

namespace eds
{
    //=====================================================================================
    // Declarations

    ///<summary><c>FormatError</c> is thrown when failed to construct a formated string.</summary>
    class FormatError : public std::runtime_error
    {
    public:
        explicit FormatError(const char *msg) : std::runtime_error(msg) { }
    };

    ///<summary>Create a formatted string in ASCII.</summary>
    ///<param name='formatter'>A text template to construct the formatted string.</param>
    ///<param name='args'>Variadic parameters referred in <c>formatter</c>.</param>
    template <typename ... TArgs>
    inline std::string Format(const char *formatter, const TArgs &...args);

    ///<summary>Create a formatted string in wide-character.</summary>
    ///<param name='formatter'>A text template to construct the formatted string.</param>
    ///<param name='args'>Variadic parameters referred in <c>formatter</c>.</param>
    template <typename ... TArgs>
    inline std::wstring Format(const wchar_t *formatter, const TArgs &...args);

    //=====================================================================================
    // Implementations
    
    namespace detail
    {
        template <typename TChar>
        using FormatArgWrapper = std::function<void(std::basic_stringstream<TChar> &)>;
        template <typename TChar>
        using FormatArgWrapperVector = static_vector<FormatArgWrapper<TChar>>;

        template <typename TChar, typename ... TArgs>
        auto GenerateArgWrappers(const TArgs &...args)
        {
            FormatArgWrapperVector<TChar> v;
            int x[] = { (v.emplace_back([&](auto &buf) { buf << args; }), 0)... };

            return v;
        }

        inline void FormatAssert(bool pred, const char *msg = "unknown format error.")
        {
            if (!pred)
            {
                throw FormatError(msg);
            }
        }

        // format string example "{}{1}"
        // "{}" => automatic incremental id in args
        // "{n}" => explicit id in args
        // no whitespace allowed in brace pair
        // double typed brace to generate raw brace character
        // currently counts of args is limited under 10
        // and only char and wchar_t are supported
        template <typename TChar, typename ... TArgs>
        std::basic_string<TChar> FormatInternal(BasicStringView<TChar> formatter, const TArgs &...args)
        {
            static_assert(sizeof...(args) < 11, "more than 10 args not supported.");

            BasicStringReader<TChar> reader{ formatter };
            // special case: formatter is empty, return to avoid some allocation
            if (reader.Exhausted())
            {
                return std::basic_string<TChar>{};
            }

            // wrap args into function delegates
            // helper[i](buf); should write (i+1)th argument into the buffer
            auto helper = GenerateArgWrappers<TChar>(args...);

            size_t next_id = 0;
            std::basic_stringstream<TChar> buf;
            while (!reader.Exhausted())
            {
                if (reader.ReadIf('{'))
                {
                    if (reader.ReadIf('{'))
                    {
                        buf.put('{');
                    }
                    else
                    {
                        size_t id;
                        if (reader.ReadIf('}'))
                        {
                            // empty brace pair
                            // id should be incremental
                            id = next_id++;
                        }
                        else
                        {
                            // id explicitly specified, should in [0, 10)
                            id = reader.Read() - '0';
                            detail::FormatAssert(id >= 0 && id <= 9, "argument id must be within [0,10).");
                            detail::FormatAssert(id < helper.size(), "not enough arguments.");

                            detail::FormatAssert(reader.ReadIf('}'), "invalid argument reference.");
                        }

                        helper[id](buf);
                    }
                }
                else if (reader.ReadIf('}'))
                {
                    if (reader.ReadIf('}'))
                    {
                        buf.put('}');
                    }
                    else
                    {
                        detail::FormatAssert(false, "an isolated closing brace is not allowed.");
                    }
                }
                else
                {
                    buf.put(reader.Read());
                }
            }

            return buf.str();
        }
    } // namespace internal

    template <typename ... TArgs>
    inline std::string Format(const char *formatter, const TArgs &...args)
    {
        
        return detail::FormatInternal<char>(StringView{ formatter }, args...);
    }

    template <typename ... TArgs>
    inline std::wstring Format(const wchar_t *formatter, const TArgs &...args)
    {
        return detail::FormatInternal<wchar_t>(WStringView{ formatter }, args...);
    }
}