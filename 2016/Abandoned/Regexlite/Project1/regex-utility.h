#pragma once
#include "regex-def.h"

namespace eds {
namespace regex {

    inline void ConstructionAssert(bool condition, const char *msg = "")
    {
        if (!condition)
        {
            throw RegexConstructionError{ msg };
        }
    }

    inline void EvaluationAssert(bool condition, const char *msg = "")
    {
        if (!condition)
        {
            throw RegexEvaluationError{ msg };
        }
    }

} // namespace regex
} // namespace eds