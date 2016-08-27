#include "regex-model.h"
#include "regex-symbol.h"
#include <functional>
#include <algorithm>

namespace eds {
namespace regex {

    //======================================================================================
    // CharSet

    symbol_t CharSet::Min() const
    {
        if (reverse_)
        {
            if (ranges_.front().min <= kMinimalCodepoint)
            {
                return ranges_.front().max;
            }
            else
            {
                return kMinimalCodepoint;
            }
        }
        else
        {
            return ranges_.front().min;
        }
    }

    symbol_t CharSet::Max() const
    {
        if (reverse_)
        {
            if (ranges_.front().max >= kMaximalCodepoint)
            {
                return ranges_.back().min;
            }
            else
            {
                return kMaximalCodepoint;
            }
        }
        else
        {
            return ranges_.back().max;
        }
    }

    void CharSet::ForEach(const std::function<void(SymbolRange)> &callback) const
    {
        if (!reverse_)
        {
            for (SymbolRange range : ranges_)
            {
                callback(range);
            }
        }
        else
        {
            symbol_t start = kMinimalCodepoint;
            for (SymbolRange range : ranges_)
            {
                if (start < range.min)
                {
                    callback(SymbolRange{ start, range.min });
                }

                start = range.max;
            }

            start = ranges_.back().max;
            if (start < kMaximalCodepoint)
            {
                callback(SymbolRange(start, kMaximalCodepoint));
            }
        }
    }

    //======================================================================================
    // SymbolDictionary

    symbol_t SymbolDictionary::Translate(symbol_t s) const
    {
        auto result_iter = std::find_if(lexemes_.begin(), lexemes_.end(),
            [s](const SymbolRange &range)
        {
            return range.Contain(s);
        });

        if (result_iter != lexemes_.end())
        {
            symbol_t result = std::distance(lexemes_.begin(), result_iter);
            
            Ensures(result != kInvalidSymbol);
            return result;
        }
        else
        {
            return kInvalidSymbol;
        }
    }

    SymbolRange SymbolDictionary::Remap(SymbolRange original_range) const
    {
        symbol_t begin, end;
        bool found = false;
        size_t index = 0;
        while (index < lexemes_.size())
        {
            if (original_range.Contain(lexemes_[index]))
            {
                // first time, set found true and record begin
                if (!found)
                {
                    found = true;
                    begin = index;
                }
            }
            else if (found)
            {
                break;
            }

            // else keep searching
            index += 1;
        }

        end = index;

        Ensures(found);
        return SymbolRange{ begin, end };
    }

    //======================================================================================
    // RangeAccumulator

    void RangeAccumulator::Insert(symbol_t ch)
    {
        Insert(SymbolRange{ ch });
    }
    void RangeAccumulator::Insert(SymbolRange range)
    {
        Expects(range.Validate());

        const auto InsertNode =
            [this](size_t value, bool left_hand)
        {
            auto comp = [](NodeType finded, int expected)
                { return finded.value < expected; };
            auto iter = std::lower_bound(
                container_.begin(), container_.end(), value, comp);

            // if not found, emplace a new one
            if (iter == container_.end() || iter->value != value)
            {
                iter = container_.emplace(iter);
                iter->value = value;
            }

            // if this note is an opening one
            if (left_hand)
            {
                iter->begin += 1;
                Ensures(iter->begin != 0);
            }
            else
            {
                iter->end += 1;
                Ensures(iter->end != 0);
            }
        };

        InsertNode(range.min, true);
        InsertNode(range.max, false);
    }

    std::vector<SymbolRange> RangeAccumulator::ExtractMerged() const
    {
        std::vector<SymbolRange> result;

        bool has_lhs = false;
        symbol_t lhs_value;
        int depth = 0;
        for (auto node : container_)
        {
            depth += node.begin;
            depth -= node.end;
            Asserts(depth >= 0);

            if (!has_lhs)
            {
                has_lhs = true;
                lhs_value = node.value;
            }
            else if (depth == 0)
            {
                // when depth is dropped from somewhere to zero
                has_lhs = false;
                result.emplace_back(lhs_value, node.value);
            }


        }

        return result;
    }

    std::vector<SymbolRange> RangeAccumulator::ExtractDisjoint() const
    {
        std::vector<SymbolRange> result;
        int last_value = 0;
        int depth = 0;
        for (auto node : container_)
        {
            Asserts(depth >= 0);

            if (depth > 0)
            {
                result.emplace_back(last_value, node.value);
            }

            last_value = node.value;
            depth += node.begin;
            depth -= node.end;
        }

        return result;
    }

} // namespace regex
} // namespace eds