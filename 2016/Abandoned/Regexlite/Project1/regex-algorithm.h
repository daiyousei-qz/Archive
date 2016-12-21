#pragma once
#include "regex-model.h"
#include "regex-automaton.h"

namespace eds {
namespace regex {

    //
    // Algorithms
    //
    bool CoreFunctionOnlyInvoker(const ExprBase *root);
    SymbolDictionary RewriteSymbolsInvoker(const ExprBase *root);
    NfaAutomaton CreateEpsilonNfaInvoker(const ExprBase *root, bool right_to_left);

} // namespace regex
} // namespace eds