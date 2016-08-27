#include "regex-model.h"
#include "regex-symbol.h"
#include "regex-def.h"
#include "regex-parser.h"
#include "regex-text.h"
#include "regex-utility.h"
#include <vector>
#include <stack>

//===================================================================================
// RegexExprBuilder

namespace
{
    using namespace eds;
    using namespace eds::regex;

    class RegexExprBuilderBase
    {
    protected:
        RegexExprBuilderBase() = default;

    public:
#pragma region MakeXXXExpr

        ExprBase* MakeConcatenationExpr(const std::vector<ExprBase*> &children)
        {
            return arena_.Construct<ConcatenationExpr>(MakeArray(children));
        }
        ExprBase* MakeAlternationExpr(const std::vector<ExprBase*> &children)
        {
            return arena_.Construct<AlternationExpr>(MakeArray(children));
        }
        ExprBase* MakeRepetitionExpr(ExprBase *child, Repetition def)
        {
            return arena_.Construct<RepetitionExpr>(child, def);
        }
        ExprBase* MakeEntityExpr(SymbolRange def)
        {
            return arena_.Construct<EntityExpr>(def);
        }
        ExprBase* MakeAnchorExpr(AnchorType def)
        {
            return arena_.Construct<AnchorExpr>(def);
        }
        ExprBase* MakeCaptureExpr(ExprBase *child, capture_t id)
        {
            return arena_.Construct<CaptureExpr>(child, id);
        }
        ExprBase* MakeReferenceExpr(capture_t id)
        {
            return arena_.Construct<ReferenceExpr>(id);
        }
        ExprBase* MakeAssertionExpr(ExprBase *child, AssertionType type)
        {
            return arena_.Construct<AssertionExpr>(child, type);
        }

#pragma endregion

        capture_t RegisterCapture(std::string name)
        {
#pragma warning()
            capture_t id = capture_list.size();
            capture_list.push_back(std::move(name));

            return id;
        }
        capture_t LookupCapture(const std::string &name)
        {
            auto item_iter = std::find(capture_list.begin(), capture_list.end(), name);
            capture_t id = std::distance(capture_list.begin(), item_iter);
            if (id != capture_list.size())
            {
                return id;
            }
            else
            {
                return kInvalidCaptureId;
            }
        }

        RegexExpr::Ptr Export(ExprBase *root)
        {
            return std::make_unique<RegexExpr>(std::move(arena_), root, std::move(capture_list));
        }

    private:
        ArrayRef<ExprBase*> MakeArray(const std::vector<ExprBase*> &children)
        {
            ArrayRef<ExprBase*> array = arena_.Allocate<ExprBase*>(children.size());
            std::copy(children.cbegin(), children.cend(), array.FrontPointer());

            return array;
        }

    private:
        Arena arena_;
        std::vector<std::string> capture_list;
    };


    // provides with standard behavior for building a regex parse tree
    // structural error should be checked and thrown by this class
    class RegexExprBuilder : RegexExprBuilderBase, Uncopyable, Unmovable
    {
    private:
        enum GroupCategory
        {
            Trivial,
            Capture,
            Assertion,
        };

        struct GroupBuffer
        {
            GroupCategory category = GroupCategory::Trivial;
            union
            {
                capture_t capture_id;
                AssertionType assertion_type;
            } info;

            std::vector<ExprBase*> alternations;
            std::vector<ExprBase*> concatenations;
        };

    public:
        RegexExprBuilder()
        {
            can_repeat = false;
            can_alternate = false;
            Initialize();
        }

    public:
#pragma region Builder Operations

        void BeginNoncaptureGroup()
        {
            // create new buffer at top of the stack
            GroupBuffer &buffer = PushNewBuffer();
            // initialize the buffer
            buffer.category = GroupCategory::Trivial;

            // update the flags
            can_alternate = false;
            can_repeat = false;
        }
        void BeginCaptureGroup(const std::string &name)
        {
            // avoid redefinition
            ConstructionAssert(LookupCapture(name) == kInvalidCaptureId, "redefinition of capture name");
            // register the name
            capture_t new_id = RegisterCapture(name);
            // create new buffer at top of the stack
            GroupBuffer &buffer = PushNewBuffer();
            // initialize the buffer
            buffer.category = GroupCategory::Capture;
            buffer.info.capture_id = new_id;

            // update the flags
            can_alternate = false;
            can_repeat = false;
        }
        void BeginAssertionGroup(AssertionType type)
        {
            // create new buffer at top of the stack
            GroupBuffer &buffer = PushNewBuffer();
            // initialize the buffer
            buffer.category = GroupCategory::Assertion;
            buffer.info.assertion_type = type;

            // update the flags
            can_alternate = false;
            can_repeat = false;
        }
        void EndLastGroup()
        {
            // ensures balanced group-call
            ConstructionAssert(GroupDepth() > 1);
            // builds topmost expression and concats it
            auto result = BuildTop();
            ConcatOnTop(result.second);

            // update the flags
            if (result.first == GroupCategory::Assertion)
            {
                can_repeat = false;
            }
            else
            {
                can_alternate = true;
                can_repeat = true;
            }
        }

        void ConcatCharRange(SymbolRange range)
        {
            ConcatOnTop(MakeEntityExpr(range));

            // update the flags
            can_alternate = true;
            can_repeat = true;
        }
        void ConcatAnchor(AnchorType anchor)
        {
            ConcatOnTop(MakeAnchorExpr(anchor));

            can_repeat = false;
        }
        void ConcatReference(const std::string &name)
        {
            // lookup id of the given capture name
            capture_t id = LookupCapture(name);
            ConstructionAssert(id != kInvalidCaptureId);
            // concat new expression
            ConcatOnTop(MakeReferenceExpr(id));
            // update the flags
            can_alternate = true;
            can_repeat = true;
        }
        void Alternate()
        {
            ConstructionAssert(can_alternate, "");

            AlternateOnTop();
            // update the flags
            can_alternate = false;
            can_repeat = false;
        }
        void RepeatLast(Repetition closure)
        {
            ConstructionAssert(can_repeat, "");

            RepeatOnTop(closure);
            // update the flags
            can_repeat = false;
        }

#pragma endregion

        RegexExpr::Ptr Build()
        {
            // ensure balanced depth
            ConstructionAssert(GroupDepth() == 1);
            // generate result
            auto result = Export(BuildTop().second);
            // reinitialize
            Initialize();

            return result;
        }

    private:
        void Initialize()
        {
            while (!groups_.empty())
            {
                groups_.pop();
            }

            groups_.emplace();
        }

        int GroupDepth() const
        {
            return groups_.size();
        }
        GroupBuffer &TopmostBuffer()
        {
            return groups_.top();
        }
        GroupBuffer &PushNewBuffer()
        {
            groups_.emplace();

            return groups_.top();
        }
        GroupBuffer DiscardTopmostBuffer()
        {
            GroupBuffer buffer = std::move(groups_.top());
            groups_.pop();

            return buffer;
        }

        void ConcatOnTop(ExprBase *expr)
        {
            Asserts(GroupDepth() > 0);

            TopmostBuffer().concatenations.push_back(expr);
        }
        void AlternateOnTop()
        {
            Asserts(GroupDepth() > 0);

            GroupBuffer &top_buffer = groups_.top();
            Asserts(top_buffer.concatenations.size() != 0);

            top_buffer.alternations.push_back(ExtractConcatenation(top_buffer));
        }
        void RepeatOnTop(Repetition closure)
        {
            Asserts(GroupDepth() > 0);
            Asserts(TopmostBuffer().concatenations.size() > 0);

            ExprBase *expr = MakeRepetitionExpr(TopmostBuffer().concatenations.back(), closure);
            TopmostBuffer().concatenations.back() = expr;
        }

        std::pair<GroupCategory, ExprBase*> BuildTop()
        {
            Asserts(GroupDepth() > 0);

            // unify the expression type
            if (TopmostBuffer().concatenations.size() != 0)
            {
                AlternateOnTop();
            }

            // retrive buffer at the top
            GroupBuffer top_buffer = DiscardTopmostBuffer();

            // generate expression model
#pragma warning()
            ConstructionAssert(top_buffer.alternations.size() != 0); // maybe this is acceptable?
            ExprBase *result = ExtractAlternation(top_buffer);

            // wrap the expression model if needed
            switch (top_buffer.category)
            {
            case GroupCategory::Assertion:
                result = MakeAssertionExpr(result, top_buffer.info.assertion_type);
                break;
            case GroupCategory::Capture:
                result = MakeCaptureExpr(result, top_buffer.info.capture_id);
                break;
            }

            return{ top_buffer.category, result };
        }

        ExprBase* ExtractConcatenation(GroupBuffer &buf)
        {
            Expects(buf.concatenations.size() != 0);

            ExprBase *result = buf.concatenations.size() > 1
                ? MakeConcatenationExpr(buf.concatenations)
                : buf.concatenations.front();

            buf.concatenations.clear();
            return result;
        }
        ExprBase* ExtractAlternation(GroupBuffer &buf)
        {
            Expects(buf.alternations.size() != 0);

            ExprBase *result = buf.alternations.size() > 1
                ? MakeAlternationExpr(buf.alternations)
                : buf.alternations.front();

            buf.alternations.clear();
            return result;
        }

    private:
        bool can_repeat;
        bool can_alternate;
        std::stack<GroupBuffer> groups_;
    };
}

//===================================================================================
// RegexTokenListener

namespace 
{
    using namespace eds;
    using namespace eds::regex;

    enum class GroupSpecifier
    {
        None,               // (...)
        Noncapture,         // (?:...)
        NamedCapture,       // (?=<name>...)
        PositiveLookAhead,  // 
        NegativeLookAhead,  //
        PositiveLookBehind, //
        NegativeLookBehind, //
    };

    // abstract regex in a sequence of tokens
    // this class only provide a adpter interface for ParseRegex
    // but doesn't do anything significant
    class RegexTokenListener : Uncopyable, Unmovable
    {
    public:
        RegexTokenListener(RegexOption option)
            : option_(option) { }
    public:
        void FeedBeginGroup(GroupSpecifier specifier, const std::string &extra_info)
        {
            switch (specifier)
            {
            case GroupSpecifier::None:
                if (option_.implicit_capture)
                {
                    BeginCaptureGroupHelper(NewDefaultCaptureName());
                }
                else
                {
                    builder_.BeginNoncaptureGroup();
                }
                break;
            case GroupSpecifier::Noncapture:
                builder_.BeginNoncaptureGroup();
                break;

            case GroupSpecifier::NamedCapture:
                BeginCaptureGroupHelper(extra_info);
                break;
            case GroupSpecifier::PositiveLookAhead:
                BeginAssertionGroupHelper(AssertionType::NegativeLookBehind);
                break;
            case GroupSpecifier::NegativeLookAhead:
                BeginAssertionGroupHelper(AssertionType::NegativeLookBehind);
                break;
            case GroupSpecifier::PositiveLookBehind:
                BeginAssertionGroupHelper(AssertionType::NegativeLookBehind);
                break;
            case GroupSpecifier::NegativeLookBehind:
                BeginAssertionGroupHelper(AssertionType::NegativeLookBehind);
                break;
            default:
                Asserts(false);
            }
        }
        void FeedEndGroup()
        {
            builder_.EndLastGroup();
        }
        void FeedAlternation()
        {
            builder_.Alternate();
        }
        void FeedQuantifier(Repetition closure)
        {
            if (closure.strategy == ClosureStrategy::Reluctant)
            {
                EnsureRichFunctional();
            }

            builder_.RepeatLast(closure);
        }

        void FeedAnchor(AnchorType anchor) 
        {
            EnsureRichFunctional();
            builder_.ConcatAnchor(anchor);
        }
        void FeedCodepoint(symbol_t value)
        {
#pragma warning()
            // more robust function should be used
            if (option_.ignore_whitespace && isspace(value))
            {
                return;
            }
            if (option_.ignore_case)
            {
                value = tolower(value);
            }

            builder_.ConcatCharRange(SymbolRange{ value });
        }
        void FeedCharSet(const CharSet &cs) 
        {
            builder_.BeginNoncaptureGroup();
            cs.ForEach(
                [&](SymbolRange range)
            {
                builder_.ConcatCharRange(range);
                builder_.Alternate();
            });
            builder_.EndLastGroup();
        }

        RegexExpr::Ptr YieldResult()
        {
            return builder_.Build();
        }
    private:
        void EnsureRichFunctional() const
        {
            ConstructionAssert(option_.matcher == MatcherType::RichNfa, "...");
        }
     
        std::string NewDefaultCaptureName()
        {
            static const char *kDecimalDigitList = "0123456789";

            capture_counter_ += 1;
            Ensures(capture_counter_ != 0);

            // anyway, capture_counter_ > 0
            size_t name_in_int = capture_counter_;
            std::string buffer;
            while (name_in_int != 0)
            {
                buffer.push_back(kDecimalDigitList[name_in_int % 10]);
                name_in_int /= 10;
            }

            return buffer;
        }

        void BeginCaptureGroupHelper(const std::string &name)
        {
            EnsureRichFunctional();
            ConstructionAssert(name.size() > 0, "empty capture name not allowed");
            builder_.BeginCaptureGroup(name);
        }
        void BeginAssertionGroupHelper(AssertionType type)
        {
            EnsureRichFunctional();
            builder_.BeginAssertionGroup(type);
        }

    private:
        const RegexOption option_;
        size_t capture_counter_ = 0;

        RegexExprBuilder builder_;
    };

}

//===================================================================================
// Parsing Regex

namespace
{
    using namespace eds;
    using namespace eds::regex;

    char32_t EscapeCharacter(char32_t ch)
    {
        switch (ch)
        {
        case 'a':
            return '\u0007';
        case 'b':
            return '\u0008';
        case 't':
            return '\u0009';
        case 'r':
            return '\u000D';
        case 'v':
            return '\u000B';
        case 'f':
            return '\u000C';
        case 'n':
            return '\u000A';
        case 'e':
            return '\u001B';
        default:
            return ch;
        }
    }
    int ParseOctalDigit(Utf8Reader &reader)
    {
        int ch = reader.Read();
        ConstructionAssert(ch >= '0' && ch <= '7', "invalid octal digit");

        return ch - '0';
    }
    int ParseDicimalDigit(Utf8Reader &reader)
    {
        int ch = reader.Read();
        ConstructionAssert(ch >= '0' && ch <= '9', "invalid decimal digit");

        return ch - '0';
    }
    int ParseHexadecimalDigit(Utf8Reader &reader)
    {
        int ch = reader.Read();
        if (ch >= '0' && ch <= '9')
        {
            return ch - '0';
        }

        ch = tolower(ch);
        if (ch >= 'a' && ch <= 'f')
        {
            return ch + 10 - 'a';
        }

        ConstructionAssert(false, "invalid hexadecimal digit");
    }
    size_t ParsePositiveInteger(Utf8Reader &reader)
    {
        static constexpr size_t kMostIterationCount = 4;

        size_t result = 0;
        size_t counter = 0;
        while (ascii::IsDigit(reader.Peek()))
        {
            if (counter > kMostIterationCount)
            {
                ConstructionAssert(false, "number is too long in characters");
            }

            // count should be in decimal form
            result = result * 10 + (reader.Read() - '0');
            counter += 1;
        }

        ConstructionAssert(counter > 0, "number must be a non-negative number");
        return result;
    }
    CharSet ParseCharClass(Utf8Reader &reader)
    {
        ConstructionAssert(reader.ReadIf('['));

        // for char class representation "[_...]"
        // assume cursor is at _
        RangeAccumulator accumulator;

        // consume a leading ^ as reverse meta char if any
        bool reverse = reader.ReadIf('^');
        // empty char class is not allowed
        ConstructionAssert(!reader.PeekIf(']'), "empty char class is not allowed");

        // if any last character can be joint into a range, i.e. as a-z
        bool joinable = false;
        char32_t last_ch;
        while (!reader.ReadIf(']'))
        {
            if (joinable && reader.ReadIf('-'))
            {
                joinable = false;

                if (!reader.PeekIf(']'))
                {
                    // case ...a-z...
                    // a range is formed
                    bool escape = reader.ReadIf('\\');
                    char32_t current_ch = reader.Read();
                    // maybe escape
                    if (escape)
                    {
                        current_ch = EscapeCharacter(current_ch);
                    }

                    // make sure last_ch <= current_ch
                    if (last_ch > current_ch)
                    {
                        std::swap(last_ch, current_ch);
                    }
                    accumulator.Insert(SymbolRange{ last_ch, current_ch + 1 });
                }
                else
                {
                    // case ...-]
                    // - is treated as a normal character
                    accumulator.Insert('-');
                }
            }
            else
            {
                joinable = true;

                reader.ReadIf('\\'); // slash to escape
                last_ch = reader.Read();
                accumulator.Insert(last_ch);
            }
        }

        return CharSet{ reverse, accumulator.ExtractMerged() };
    }

    static const CharSet kAnyCharSet = CharSet{ false, SymbolRange{ 1, 0x10ffff } };
    static const CharSet kWordCharSet = ParseCharClass(Utf8Reader{ "[a-zA-Z]" });
    static const CharSet kReversedWordCharSet = ParseCharClass(Utf8Reader{ "[^a-zA-Z]" });
    static const CharSet kDigitCharSet = ParseCharClass(Utf8Reader{ "[0-9]" });
    static const CharSet kReversedDigitCharSet = ParseCharClass(Utf8Reader{ "[^0-9]" });
    static const CharSet kWhitespaceCharSet = ParseCharClass(Utf8Reader{ "[ \t\r\n]" });
    static const CharSet kReversedWhitespaceCharSet = ParseCharClass(Utf8Reader{ "[^ \t\r\n]" });

#pragma region ParseTokenXXX

    void ParseTokenBeginGroup(Utf8Reader &reader, RegexTokenListener &listener)
    {
        ConstructionAssert(reader.ReadIf('('));

        GroupSpecifier specifier = GroupSpecifier::None;
        std::string extra_info;
        if (reader.ReadIf("?:"))
        {
            specifier = GroupSpecifier::Noncapture;
        }
        else if (reader.ReadIf("?="))
        {
            specifier = GroupSpecifier::PositiveLookAhead;
        }
        else if (reader.ReadIf("?!"))
        {
            specifier = GroupSpecifier::NegativeLookAhead;
        }
        else if (reader.ReadIf("?<="))
        {
            specifier = GroupSpecifier::PositiveLookBehind;
        }
        else if (reader.ReadIf("?<!"))
        {
            specifier = GroupSpecifier::NegativeLookBehind;
        }
        else if (reader.ReadIf("?<"))
        {
            // NOTE this test maybe overlap with one with PositiveLookBehind
            // so it must be later

            size_t offset_begin = reader.Cursor();
            // simply advance the reader until '>'
            while (reader.Read() != '>');
            size_t offset_end = reader.Cursor();

            StringView name_view = reader.BaseString().SubString(offset_begin, offset_end - offset_begin);

            specifier = GroupSpecifier::NamedCapture;
            extra_info = name_view.ToString();
        }

        listener.FeedBeginGroup(specifier, extra_info);
    }
    void ParseTokenRepeat(Utf8Reader &reader, RegexTokenListener &listener)
    {
        // NOTE that whitespace is not allowed in explicit closure definition
        Repetition::CountType least, most;
        ClosureStrategy strategy;

        // parse repetition count
        char32_t ch = reader.Read();
        switch (ch)
        {
        case '?':
            least = 0;
            most = 1;
            break;
        case '*':
            least = 0;
            most = Repetition::kInifinity;
            break;
        case '+':
            least = 1;
            most = Repetition::kInifinity;
            break;
        case '{':
            // empty explicit closure, ie.{}, is not allowed
            ConstructionAssert(!reader.PeekIf('}'), "empty closure definition, i.e. {} is not allowed");

            // least must be provided
            least = ParsePositiveInteger(reader);

            // default for most is exactly least, note {n}
            most = least;
            if (reader.ReadIf(','))
            {
                // read second count if any, maybe infinity
                if (reader.PeekIf('}'))
                {
                    // {n,}
                    most = Repetition::kInifinity;
                }
                else
                {
                    // {n,m}
                    most = ParsePositiveInteger(reader);
                }
            }

            // enclosing brace
            ConstructionAssert(reader.Read() == '}', "brace must be closed in closure definition");
            break;

        } // switch

          // parse repetition strategy
        strategy = reader.ReadIf('?')
            ? ClosureStrategy::Reluctant
            : ClosureStrategy::Greedy;

        listener.FeedQuantifier(Repetition{ least, most, strategy });
    }
    void ParseTokenEscaped(Utf8Reader &reader, RegexTokenListener &listener)
    {
        // read the leading back slash
        ConstructionAssert(reader.ReadIf('\\'));
        // dispatch behavior according to the first character
        // right after the back slash
        char32_t ch = reader.Read();
        switch (ch)
        {
        // character class
        case 'w':
            listener.FeedCharSet(kWordCharSet);
            break;
        case 'W':
            listener.FeedCharSet(kReversedWordCharSet);
            break;
        case 'd':
            listener.FeedCharSet(kDigitCharSet);
            break;
        case 'D':
            listener.FeedCharSet(kReversedDigitCharSet);
            break;
        case 's':
            listener.FeedCharSet(kWhitespaceCharSet);
            break;
        case 'S':
            listener.FeedCharSet(kReversedWhitespaceCharSet);
            break;

        // unicode properties
        case 'p':
        case 'P':
            throw 0;
            break;

        // assertion
        //case 'b':
        case 'B':
        case 'A':
        case 'Z':
        case 'z':
        case 'G':
            throw 0;
            break;

        // backreference
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            throw 0;
            break;
        case 'k':
            throw 0;
            break;
        // explicit codepoint
        case '0':
        {
            symbol_t codepoint = ch;
            for (int i = 0; i < 2; ++i)
            {
                codepoint << 3;
                codepoint |= ParseOctalDigit(reader);
            }

            listener.FeedCodepoint(codepoint);
            break;
        }
        case 'x':
        {
            symbol_t codepoint = 0;
            for (int i = 0; i < 2; ++i)
            {
                codepoint << 4;
                codepoint |= ParseHexadecimalDigit(reader);
            }

            listener.FeedCodepoint(codepoint);
            break;
        }
        case 'u':
        {
            symbol_t codepoint = 0;
            for (int i = 0; i < 4; ++i)
            {
                codepoint << 4;
                codepoint |= ParseHexadecimalDigit(reader);
            }

            listener.FeedCodepoint(codepoint);
            break;
        }
        case 'c':
#pragma warning()
            throw 0;
            break;

        default:
            listener.FeedCodepoint(EscapeCharacter(ch));
            break;
        }
    }

#pragma endregion

    void ParseNextToken(Utf8Reader &reader, RegexTokenListener &listener)
    {
        Asserts(!reader.Exhausted());

        // parse next token and feed the listener according to the leading character
        switch (reader.Peek())
        {
        case '(':
            ParseTokenBeginGroup(reader, listener);
            break;
        case ')':
            reader.Read();
            listener.FeedEndGroup();
            break;
        case '|':
            reader.Read();
            listener.FeedAlternation();
            break;
        case '?':
        case '*':
        case '+':
        case '{':
            ParseTokenRepeat(reader, listener);
            break;
        case '\\':
            ParseTokenEscaped(reader, listener);
            break;
        case '[':
            listener.FeedCharSet(ParseCharClass(reader));
            break;
        case '.':
            reader.Read();
            listener.FeedCharSet(kAnyCharSet);
            break;
        case '$':
            reader.Read();
            listener.FeedAnchor(AnchorType::Dollar);
            break;
        case '^':
            reader.Read();
            listener.FeedAnchor(AnchorType::Circumflex);
            break;

        default:
            listener.FeedCodepoint(reader.Read());
            break;
        }
    }
    void ParseRegexInternal(StringView regex, RegexTokenListener &listener)
    {
        // implementation for parsing a regex
        // this method doesn't track states of parsing
        // if given regex is invalid in logic, exceptions should be thrown by the listener
        try
        {
            Utf8Reader reader{ regex };
            while (!reader.Exhausted())
            {
                ParseNextToken(reader, listener);
            }
        }
        catch (const UnexpectedEOSError &)
        {
            ConstructionAssert(false, "unexpected EOS");
        }
        catch (const BadEncodingError &)
        {
            ConstructionAssert(false, "bad encoding in utf8");
        }
    }
}

namespace eds {
namespace regex {

    RegexExpr::Ptr ParseRegex(StringView regex, RegexOption option)
    {
        RegexTokenListener listener{ option };
        ParseRegexInternal(regex, listener);

        return listener.YieldResult();
    }

} // namespace regex
} // namespace eds
