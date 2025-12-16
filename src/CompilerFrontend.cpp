#include "CompilerFrontend.h"
#include "CalculatorLanguage.h"
#include "CompilerException.h"
#include <iomanip>
#include <iostream>

// Typedef for clarity, matching the header usage of PDA
using PDASyntaxValidator = PDA;

CompilerFrontend::CompilerFrontend()
    : syntaxValidator(CalculatorLanguage::createPDA()) // Initialize PDA
{
  // Initialize Lexer
  CalculatorLanguage::configureLexer(lexer);
}

void CompilerFrontend::run(const std::string &sourceCode) {
  std::cout << "\n=== Compiler Front-End Simulation ===\n";
  std::cout << "Input: " << sourceCode << "\n\n";

  // --- 2. Lexical Analysis Layer ---
  std::vector<Token> tokens;
  try {
    tokens = lexer.tokenize(sourceCode);
    printLexicalAnalysisResult(tokens);
  } catch (const CompilerException &e) {
    std::cout << "Lexical Analysis: FAILED\n";
    std::cerr << "Error: " << e.what() << "\n";
    return;
  } catch (const std::exception &e) {
    std::cout << "Lexical Analysis: FAILED\n";
    std::cerr << "Unexpected Error: " << e.what() << "\n";
    return;
  }

  // --- 3. Syntax Analysis Layer ---
  std::cout << "\n=== SYNTACTIC ANALYSIS: Pushdown Automaton ===\n\n";
  std::cout
      << "Grammar:\n  E -> T E'\n  E' -> + T E' | - T E' | epsilon\n  T -> F "
         "T'\n  T' -> * F T' | / F T' | epsilon\n  F -> ( E ) | id | num\n\n";
  std::cout << "Input: " << sourceCode << "\n\n";

  std::cout << "[1] Regex / NFA: Not applicable for PDA (uses stack instead)\n";
  std::cout << "[2] Thompson NFA: Not applicable\n";
  std::cout << "[3] DFA Minimization: Not applicable\n";
  std::cout << "[4] PDA Stack Simulation:\n\n";

  // Extract token types for PDA
  std::vector<std::string> tokenTypes;
  for (const auto &t : tokens) {
    tokenTypes.push_back(t.type);
  }

  try {
    // Run PDA Simulation
    bool isValid =
        syntaxValidator.simulate(tokenTypes, true); // true for debug trace
    printSyntaxAnalysisResult(isValid);
  } catch (const std::exception &e) {
    std::cout << "Syntax Analysis: ERROR\n";
    std::cerr << "Error: " << e.what() << "\n";
  }
}

void CompilerFrontend::printLexicalAnalysisResult(
    const std::vector<Token> &tokens) {
  std::cout << "Lexical Analysis: SUCCESS\n";
  std::cout << "Token Stream:\n";
  for (const auto &t : tokens) {
    std::cout << "Token: " << t.type << " (" << t.value << ")\n";
  }
  std::cout << "\n";
}

void CompilerFrontend::printSyntaxAnalysisResult(bool isValid) {
  if (isValid) {
    std::cout << "Syntax Analysis: VALID EXPRESSION\n";
  } else {
    std::cout << "Syntax Analysis: INVALID EXPRESSION\n";
  }
}
