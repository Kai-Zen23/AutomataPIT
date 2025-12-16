#ifndef CALCULATORLANGUAGE_H
#define CALCULATORLANGUAGE_H

#include "Lexer.h"
#include "PDA.h"

// This class encapsulates the definitions for the Calculator Language
// It corresponds to the "Regular Expressions" and "Context-Free Grammar" blocks
// in the architecture diagram.
class CalculatorLanguage {
public:
  static void configureLexer(Lexer &lexer);
  static PDA createPDA();
};

#endif // CALCULATORLANGUAGE_H
