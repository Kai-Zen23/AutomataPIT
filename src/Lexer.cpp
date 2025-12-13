#include "Lexer.h"
#include "CompilerException.h"
#include <iostream>

void Lexer::addRule(std::string type, DFA dfa) {
  tokenRules.push_back({type, std::move(dfa)});
}

std::vector<Token> Lexer::tokenize(std::string input) {
  std::vector<Token> tokens;
  size_t pos = 0;

  while (pos < input.length()) {
    // Skip whitespace
    if (isspace(input[pos])) {
      pos++;
      continue;
    }

    std::string bestType = "";
    std::string bestMatch = "";
    size_t bestLen = 0;

    // Try to match all rules
    for (auto &rule : tokenRules) {
      std::string type = rule.first;
      DFA &dfa = rule.second;

      State *current = dfa.startState;
      size_t currentLen = 0;
      size_t lastAcceptLen = 0;

      // Simulate DFA on remaining input
      for (size_t i = pos; i < input.length(); ++i) {
        char c = input[i];
        bool moved = false;

        auto range = current->transitions.equal_range(c);
        if (range.first != range.second) {
          // DFA is deterministic, so only one transition
          current = range.first->second;
          currentLen++;
          if (current->isAccepting) {
            lastAcceptLen = currentLen;
          }
          moved = true;
        }

        if (!moved)
          break;
      }

      if (lastAcceptLen > bestLen) {
        bestLen = lastAcceptLen;
        bestType = type;
        bestMatch = input.substr(pos, bestLen);
      }
    }

    if (bestLen > 0) {
      tokens.push_back({bestType, bestMatch});
      pos += bestLen;
    } else {
      throw CompilerException("Lexer Error: Unknown token '" +
                                  std::string(1, input[pos]) + "'",
                              pos);
    }
  }

  return tokens;
}
