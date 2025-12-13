#ifndef LEXER_H
#define LEXER_H

#include "DFA.h"
#include <string>
#include <utility>
#include <vector>

struct Token {
  std::string type;
  std::string value;
};

class Lexer {
public:
  // Pairs of (TokenType, DFA)
  std::vector<std::pair<std::string, DFA>> tokenRules;

  void addRule(std::string type, DFA dfa);

  // Tokenize the input string
  std::vector<Token> tokenize(std::string input);
};

#endif // LEXER_H
