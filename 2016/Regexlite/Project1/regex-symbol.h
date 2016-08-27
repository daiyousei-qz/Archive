#pragma once
#include "regex-model.h"
#include <vector>
#include <functional>

namespace eds {
namespace regex{

    // character class representation
    // a set of unicode codepoints
    class CharSet
    {
    public:
        static constexpr symbol_t kMinimalCodepoint = 0;
        static constexpr symbol_t kMaximalCodepoint = 0x10ffff;

    public:
        CharSet(bool reverse, SymbolRange range)
            : reverse_(reverse), ranges_({ range })
        {
            Expects(range.Validate());
        }
        CharSet(bool reverse, std::vector<SymbolRange> ranges)
            : reverse_(reverse), ranges_(std::move(ranges))
        {
#pragma warning()
            // validate ranges
        }

    public:
        symbol_t Min() const;
        symbol_t Max() const;
        void ForEach(const std::function<void(SymbolRange)> &callback) const;

    private:
        bool reverse_;
        std::vector<SymbolRange> ranges_;
    };

    // SymbolRange -> [index]
    class SymbolDictionary
    {
    public:
        SymbolDictionary(std::vector<SymbolRange> definition)
            : lexemes_(std::move(definition)) 
        {
#pragma warning()
            // validate
        }

    public:
        size_t LexemeCount() const
        {
            return lexemes_.size();
        }

        symbol_t Translate(symbol_t origin) const;
        SymbolRange Remap(SymbolRange origin) const;

    private:
        std::vector<SymbolRange> lexemes_;
    };

    class RangeAccumulator
    {
    private:
        struct NodeType
        {
            uint16_t begin = 0;
            uint16_t end   = 0;
            symbol_t value = 0;
        };

    public:
        void Clear()
        {
            container_.clear();
        }

        void Insert(symbol_t ch);
        void Insert(SymbolRange range);

        std::vector<SymbolRange> ExtractMerged() const;
        std::vector<SymbolRange> ExtractDisjoint() const;

    private:
        std::vector<NodeType> container_;
    };

} // namespace eds
} // namespace regex