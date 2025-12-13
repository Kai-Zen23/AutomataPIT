#ifndef REGEXPARSER_H
#define REGEXPARSER_H

#include "NFA.h"
#include <string>
#include <vector>

class RegexParser {
public:
    RegexParser();
    NFA parse(std::string regex);

private:
    int stateCounter;
    
    // Helper to insert explicit concatenation operators
    std::string preprocessRegex(std::string regex);
    
    // Convert infix regex to postfix
    std::string toPostfix(std::string regex);
    
    // Check if character is an operator
    bool isOperator(char c);
    
    // Get operator precedence
    int getPrecedence(char c);

    // Expand ranges like [a-z] to (a|b|...|z)
    std::string expandRanges(std::string regex);
};

#endif // REGEXPARSER_H
