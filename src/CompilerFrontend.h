#ifndef COMPILERFRONTEND_H
#define COMPILERFRONTEND_H

#include "Lexer.h"
#include "PDA.h"
#include <string>
#include <vector>

// Orchestrates the "Front End" simulation
class CompilerFrontend {
public:
  CompilerFrontend();

  // The main flow: Input -> Lexer -> Syntax -> Output
  void run(const std::string &sourceCode);

private:
  Lexer lexer;
  PDASyntaxValidator syntaxValidator; // Using PDA as the validator

  void printLexicalAnalysisResult(const std::vector<Token> &tokens);
  void printSyntaxAnalysisResult(bool isValid);
};

#endif // COMPILERFRONTEND_H
