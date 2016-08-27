#pragma once
#include "regex-def.h"
#include "regex-model.h"
#include "regex-algorithm.h"
#include "string.hpp"
#include "arena.hpp"
#include <memory>

namespace eds {
namespace regex {

    class RegexExpr : Uncopyable, Unmovable
    {
    public:
        using Ptr = std::unique_ptr<RegexExpr>;
        using CaptureList = std::vector<std::string>;

    public:
        RegexExpr(Arena guard, ExprBase *root, CaptureList captures)
            : arena_(std::move(guard))
            , root_(root)
            , captures_(std::move(captures)) { }

    public:
        const ExprBase *Root()
        {
            return root_;
        }

        const CaptureList &CaptureDef()
        {
            return captures_;
        }

    private:
        Arena arena_;
        ExprBase *root_;
        CaptureList captures_;
    };

    

    RegexExpr::Ptr ParseRegex(StringView regex, RegexOption option);

} // namespace regex
} // namespace eds