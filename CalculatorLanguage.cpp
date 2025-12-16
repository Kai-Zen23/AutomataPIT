#include "CalculatorLanguage.h"
#include "DFA.h"
#include "RegexParser.h"
#include <iostream>


void CalculatorLanguage::configureLexer(Lexer &lexer) {
  RegexParser parser;
  std::cout << "[Lexical Layer] Configuring Regular Expressions and Automata..."
            << std::endl;

  try {
    // Helper lambda to add a rule
    auto addRule = [&](std::string name, std::string regex) {
      // 2a. Regular Expression
      NFA nfa = parser.parse(regex);
      // 2b. NFA
      DFA dfa = DFA::fromNFA(std::move(nfa));
      // 2c. DFA
      dfa.minimize();
      // 2d. Tokenizer Rule
      lexer.addRule(name, std::move(dfa));
    };

    addRule("NUMBER", "[0-9]+(\\.[0-9]+)?");
    addRule("IDENTIFIER", "[a-zA-Z_][a-zA-Z0-9_]*");
    addRule("PLUS", "\\+");
    addRule("MINUS", "\\-");
    addRule("MUL", "\\*");
    addRule("DIV", "/");
    addRule("LPAREN", "\\(");
    addRule("RPAREN", "\\)");
    addRule("EQUALS", "=");

  } catch (const std::exception &e) {
    std::cerr << "Error configuring lexer: " << e.what() << std::endl;
    throw;
  }
}

PDA CalculatorLanguage::createPDA() {
  std::cout << "[Syntax Layer] Configuring Context-Free Grammar (PDA)..."
            << std::endl;

  // 3b. Pushdown Automaton (PDA) setup
  // Grammar:
  // S -> IDENTIFIER = E | E
  // E -> T E'
  // E' -> + T E' | - T E' | epsilon
  // T -> F T'
  // T' -> * F T' | / F T' | epsilon
  // F -> ( E ) | NUMBER | IDENTIFIER

  PDA pda(0, "Z");
  pda.addAcceptState(1);
  const std::string EPSILON = "EPSILON";

  // Start: Push S
  pda.addTransition(0, EPSILON, "Z", 0, {"S", "Z"});

  // S -> IDENTIFIER = E
  pda.addTransition(0, EPSILON, "S", 0, {"IDENTIFIER", "EQUALS", "E"});
  // S -> E
  pda.addTransition(0, EPSILON, "S", 0, {"E"});

  // E -> T E'
  pda.addTransition(0, EPSILON, "E", 0, {"T", "E'"});

  // E' -> + T E'
  pda.addTransition(0, EPSILON, "E'", 0, {"PLUS", "T", "E'"});
  // E' -> - T E'
  pda.addTransition(0, EPSILON, "E'", 0, {"MINUS", "T", "E'"});
  // E' -> epsilon
  pda.addTransition(0, EPSILON, "E'", 0, {});

  // T -> F T'
  pda.addTransition(0, EPSILON, "T", 0, {"F", "T'"});

  // T' -> * F T'
  pda.addTransition(0, EPSILON, "T'", 0, {"MUL", "F", "T'"});
  // T' -> / F T'
  pda.addTransition(0, EPSILON, "T'", 0, {"DIV", "F", "T'"});
  // T' -> epsilon
  pda.addTransition(0, EPSILON, "T'", 0, {});

  // F -> ( E )
  pda.addTransition(0, EPSILON, "F", 0, {"LPAREN", "E", "RPAREN"});
  // F -> NUMBER
  pda.addTransition(0, EPSILON, "F", 0, {"NUMBER"});
  // F -> IDENTIFIER
  pda.addTransition(0, EPSILON, "F", 0, {"IDENTIFIER"});

  // Terminal matching
  pda.addTransition(0, "PLUS", "PLUS", 0, {});
  pda.addTransition(0, "MINUS", "MINUS", 0, {});
  pda.addTransition(0, "MUL", "MUL", 0, {});
  pda.addTransition(0, "DIV", "DIV", 0, {});
  pda.addTransition(0, "LPAREN", "LPAREN", 0, {});
  pda.addTransition(0, "RPAREN", "RPAREN", 0, {});
  pda.addTransition(0, "NUMBER", "NUMBER", 0, {});
  pda.addTransition(0, "IDENTIFIER", "IDENTIFIER", 0, {});
  pda.addTransition(0, "EQUALS", "EQUALS", 0, {});

  // Finish
  pda.addTransition(0, EPSILON, "Z", 1, {"Z"});

  return pda;
}
