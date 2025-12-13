#include "CompilerException.h"
#include "DFA.h"
#include "Lexer.h"
#include "PDA.h"
#include "RegexParser.h"
#include <iostream>

// Special constant for Epsilon in string form
const std::string EPSILON_STR = "EPSILON";

void testCalculator() {
  std::cout << "Initializing Calculator Lexer..." << std::endl;
  RegexParser parser;
  Lexer lexer;

  try {
    // Numbers
    NFA nfaNum = parser.parse("[0-9]+(\\.[0-9]+)?");
    DFA dfaNum = DFA::fromNFA(std::move(nfaNum));
    dfaNum.minimize();
    lexer.addRule("NUMBER", std::move(dfaNum));

    // Identifiers
    NFA nfaId = parser.parse("[a-zA-Z_][a-zA-Z0-9_]*");
    DFA dfaId = DFA::fromNFA(std::move(nfaId));
    dfaId.minimize();
    lexer.addRule("IDENTIFIER", std::move(dfaId));

    // Operators (Split for precedence)
    NFA nfaPlus = parser.parse("\\+");
    DFA dfaPlus = DFA::fromNFA(std::move(nfaPlus));
    dfaPlus.minimize();
    lexer.addRule("PLUS", std::move(dfaPlus));

    NFA nfaMinus = parser.parse("\\-");
    DFA dfaMinus = DFA::fromNFA(std::move(nfaMinus));
    dfaMinus.minimize();
    lexer.addRule("MINUS", std::move(dfaMinus));

    NFA nfaMul = parser.parse("\\*");
    DFA dfaMul = DFA::fromNFA(std::move(nfaMul));
    dfaMul.minimize();
    lexer.addRule("MUL", std::move(dfaMul));

    NFA nfaDiv = parser.parse("/");
    DFA dfaDiv = DFA::fromNFA(std::move(nfaDiv));
    dfaDiv.minimize();
    lexer.addRule("DIV", std::move(dfaDiv));

    // Parentheses
    NFA nfaLParen = parser.parse("\\(");
    DFA dfaLParen = DFA::fromNFA(std::move(nfaLParen));
    dfaLParen.minimize();
    lexer.addRule("LPAREN", std::move(dfaLParen));

    NFA nfaRParen = parser.parse("\\)");
    DFA dfaRParen = DFA::fromNFA(std::move(nfaRParen));
    dfaRParen.minimize();
    lexer.addRule("RPAREN", std::move(dfaRParen));
  } catch (const std::exception &e) {
    std::cerr << "Lexer Setup Error: " << e.what() << std::endl;
    return;
  }

  // PDA Construction
  // E -> T E'
  // E' -> + T E' | - T E' | epsilon
  // T -> F T'
  // T' -> * F T' | / F T' | epsilon
  // F -> ( E ) | NUMBER | IDENTIFIER

  PDA pdaCalc(0, "Z");
  pdaCalc.addAcceptState(1);

  // Start: Push E
  pdaCalc.addTransition(0, EPSILON_STR, "Z", 0, {"E", "Z"});

  // E -> T E'
  pdaCalc.addTransition(0, EPSILON_STR, "E", 0, {"T", "E'"});

  // E' -> + T E'
  pdaCalc.addTransition(0, EPSILON_STR, "E'", 0, {"PLUS", "T", "E'"});
  // E' -> - T E'
  pdaCalc.addTransition(0, EPSILON_STR, "E'", 0, {"MINUS", "T", "E'"});
  // E' -> epsilon
  pdaCalc.addTransition(0, EPSILON_STR, "E'", 0, {});

  // T -> F T'
  pdaCalc.addTransition(0, EPSILON_STR, "T", 0, {"F", "T'"});

  // T' -> * F T'
  pdaCalc.addTransition(0, EPSILON_STR, "T'", 0, {"MUL", "F", "T'"});
  // T' -> / F T'
  pdaCalc.addTransition(0, EPSILON_STR, "T'", 0, {"DIV", "F", "T'"});
  // T' -> epsilon
  pdaCalc.addTransition(0, EPSILON_STR, "T'", 0, {});

  // F -> ( E )
  pdaCalc.addTransition(0, EPSILON_STR, "F", 0, {"LPAREN", "E", "RPAREN"});
  // F -> NUMBER
  pdaCalc.addTransition(0, EPSILON_STR, "F", 0, {"NUMBER"});
  // F -> IDENTIFIER
  pdaCalc.addTransition(0, EPSILON_STR, "F", 0, {"IDENTIFIER"});

  // Terminals (Match input token type with stack symbol)
  pdaCalc.addTransition(0, "PLUS", "PLUS", 0, {});
  pdaCalc.addTransition(0, "MINUS", "MINUS", 0, {});
  pdaCalc.addTransition(0, "MUL", "MUL", 0, {});
  pdaCalc.addTransition(0, "DIV", "DIV", 0, {});
  pdaCalc.addTransition(0, "LPAREN", "LPAREN", 0, {});
  pdaCalc.addTransition(0, "RPAREN", "RPAREN", 0, {});
  pdaCalc.addTransition(0, "NUMBER", "NUMBER", 0, {});
  pdaCalc.addTransition(0, "IDENTIFIER", "IDENTIFIER", 0, {});

  // Finish
  pdaCalc.addTransition(0, EPSILON_STR, "Z", 1, {"Z"});

  std::string input;
  std::cout << "Enter Calculator String (e.g., x = 5 + 3 * y): ";
  if (std::getline(std::cin, input)) {
    if (input.empty())
      return;

    try {
      std::cout << "Tokenizing..." << std::endl;
      std::vector<Token> tokens = lexer.tokenize(input);

      std::vector<std::string> tokenTypes;
      for (const auto &t : tokens) {
        std::cout << "Token: " << t.type << " (" << t.value << ")" << std::endl;
        tokenTypes.push_back(t.type);
      }

      std::cout << "Parsing..." << std::endl;
      bool result = pdaCalc.simulate(tokenTypes, true);
      if (result)
        std::cout << "Syntax: Valid" << std::endl;
      else
        std::cout << "Syntax: Invalid" << std::endl;
    } catch (const CompilerException &e) {
      std::cerr << "Error: " << e.what() << std::endl;
    } catch (const std::exception &e) {
      std::cerr << "Unexpected Error: " << e.what() << std::endl;
    }
  }
}

void testPDA() {
  // Balanced Parentheses PDA
  PDA pda(0, "Z");
  pda.addAcceptState(1);

  // Push X on '('
  pda.addTransition(0, "(", "Z", 0, {"X", "Z"});
  pda.addTransition(0, "(", "X", 0, {"X", "X"});

  // Pop X on ')'
  pda.addTransition(0, ")", "X", 0, {});

  // Accept if stack is Z (marker)
  pda.addTransition(0, EPSILON_STR, "Z", 1, {"Z"});

  std::string input;
  std::cout << "Enter string to check (e.g., (())): ";
  if (std::getline(std::cin, input)) {
    // Tokenize char by char for this simple test
    std::vector<std::string> tokens;
    for (char c : input)
      tokens.push_back(std::string(1, c));

    bool result = pda.simulate(tokens, true);
    if (result)
      std::cout << "Result: ACCEPTED" << std::endl;
    else
      std::cout << "Result: REJECTED" << std::endl;
  }
}

int main() {
  std::string mode;
  std::cout << "Select Mode (1: Regex/DFA, 2: PDA Test, 3: Calculator): ";
  if (!std::getline(std::cin, mode))
    return 0;

  if (mode == "2") {
    testPDA();
    return 0;
  }
  if (mode == "3") {
    testCalculator();
    return 0;
  }

  RegexParser parser;
  std::string regex;

  std::cout << "Enter Regex: ";
  if (std::getline(std::cin, regex)) {
    if (regex.empty())
      return 0;

    try {
      std::cout << "Parsing regex: " << regex << std::endl;
      NFA nfa = parser.parse(regex);

      std::cout << "NFA Constructed:" << std::endl;
      nfa.print();
      nfa.toDot("nfa.dot", "NFA: " + regex);

      std::cout << "\nConverting to DFA..." << std::endl;
      DFA dfa = DFA::fromNFA(std::move(nfa));
      dfa.print();
      dfa.toDot("dfa.dot", "DFA: " + regex);

      std::cout << "\nMinimizing DFA..." << std::endl;
      dfa.minimize();
      dfa.print();
      dfa.toDot("min_dfa.dot", "Minimized DFA: " + regex);

      // Destructors handle cleanup automatically
    } catch (const CompilerException &e) {
      std::cerr << "Error: " << e.what() << std::endl;
    } catch (const std::exception &e) {
      std::cerr << "Unexpected Error: " << e.what() << std::endl;
    }
  }
  return 0;
}
