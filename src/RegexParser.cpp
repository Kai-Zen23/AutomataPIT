#include "RegexParser.h"
#include "CompilerException.h"
#include <algorithm>
#include <iostream>
#include <stack>

RegexParser::RegexParser() : stateCounter(0) {}

bool RegexParser::isOperator(char c) {
  return c == '|' || c == '.' || c == '*' || c == '+' || c == '?' || c == '(' ||
         c == ')';
}

int RegexParser::getPrecedence(char c) {
  if (c == '|')
    return 1;
  if (c == '.')
    return 2;
  if (c == '*' || c == '+' || c == '?')
    return 3;
  return 0;
}

std::string RegexParser::expandRanges(std::string regex) {
  std::string result = "";
  for (size_t i = 0; i < regex.length(); ++i) {
    if (regex[i] == '[') {
      i++;
      std::string rangeContent = "";
      while (i < regex.length() && regex[i] != ']') {
        rangeContent += regex[i];
        i++;
      }

      std::string expanded = "(";
      for (size_t j = 0; j < rangeContent.length(); ++j) {
        if (j + 2 < rangeContent.length() && rangeContent[j + 1] == '-') {
          char start = rangeContent[j];
          char end = rangeContent[j + 2];
          for (char c = start; c <= end; ++c) {
            expanded += c;
            if (c < end)
              expanded += '|';
          }
          j += 2;
        } else {
          if (isOperator(rangeContent[j])) {
            expanded += '\\';
          }
          expanded += rangeContent[j];
        }
        if (j < rangeContent.length() - 1 && rangeContent[j + 1] != '-') {
          expanded += '|';
        }
      }
      if (expanded.back() == '|')
        expanded.pop_back();
      expanded += ")";
      result += expanded;
    } else {
      result += regex[i];
    }
  }
  return result;
}

std::string RegexParser::preprocessRegex(std::string regex) {
  std::string expanded = expandRanges(regex);
  std::string result = "";

  for (size_t i = 0; i < expanded.length(); ++i) {
    char c1 = expanded[i];

    if (c1 == '\\') {
      result += c1;
      if (i + 1 < expanded.length()) {
        result += expanded[++i];
      }
      // Escaped char is an operand
      if (i + 1 < expanded.length()) {
        char c2 = expanded[i + 1];
        bool c2IsOperand = !isOperator(c2) || c2 == '(' || c2 == '\\';
        // Note: if c2 is '\', it starts an operand.
        // But wait, isOperator('\\') is false. So !isOperator is true.
        // So c2IsOperand is true.

        if (c2IsOperand) {
          result += '.';
        }
      }
      continue;
    }

    result += c1;

    if (i + 1 < expanded.length()) {
      char c2 = expanded[i + 1];

      bool c1IsOperand =
          !isOperator(c1) || c1 == ')' || c1 == '*' || c1 == '+' || c1 == '?';
      bool c2IsOperand = !isOperator(c2) || c2 == '(';

      // Special case: if c2 is '\', it starts an operand (escaped char)
      if (c2 == '\\')
        c2IsOperand = true;

      if (c1IsOperand && c2IsOperand) {
        result += '.';
      }
    }
  }
  return result;
}

std::string RegexParser::toPostfix(std::string regex) {
  std::string postfix = "";
  std::stack<char> operators;

  for (size_t i = 0; i < regex.length(); ++i) {
    char c = regex[i];

    if (c == '\\') {
      postfix += c;
      if (i + 1 < regex.length()) {
        postfix += regex[++i];
      }
      continue;
    }

    if (!isOperator(c)) {
      postfix += c;
    } else if (c == '(') {
      operators.push(c);
    } else if (c == ')') {
      while (!operators.empty() && operators.top() != '(') {
        postfix += operators.top();
        operators.pop();
      }
      if (operators.empty()) {
        throw CompilerException("Regex Error: Unbalanced parentheses");
      }
      operators.pop(); // Pop '('
    } else {
      while (!operators.empty() &&
             getPrecedence(operators.top()) >= getPrecedence(c)) {
        postfix += operators.top();
        operators.pop();
      }
      operators.push(c);
    }
  }

  while (!operators.empty()) {
    if (operators.top() == '(') {
      throw CompilerException("Regex Error: Unbalanced parentheses");
    }
    postfix += operators.top();
    operators.pop();
  }

  return postfix;
}

NFA RegexParser::parse(std::string regex) {
  std::string preprocessed = preprocessRegex(regex);
  std::string postfix = toPostfix(preprocessed);

  std::cout << "\n=== LEXICAL ANALYSIS: Regular Expression -> NFA ===\n\n";
  std::cout << "[1] Input Pattern:\n  " << regex << "\n\n";

  std::cout << "[2] Character Class Expansion & Preprocessing:\n  "
            << preprocessed << "\n";
  std::cout << "  (Character classes expanded to unions)\n\n";

  std::cout << "[3] Postfix Notation (RPN):\n  " << postfix << "\n";
  std::cout << "  (Ready for Thompson's NFA construction)\n\n";

  std::stack<NFA> nfaStack;

  for (size_t i = 0; i < postfix.length(); ++i) {
    char c = postfix[i];

    if (c == '\\') {
      if (i + 1 < postfix.length()) {
        char escapedChar = postfix[++i];
        nfaStack.push(NFA::fromChar(escapedChar, stateCounter));
      }
      continue;
    }

    if (!isOperator(c)) {
      nfaStack.push(NFA::fromChar(c, stateCounter));
    } else if (c == '.') {
      if (nfaStack.size() < 2) {
        throw CompilerException(
            "Regex Error: Stack underflow for concatenation operator '.'");
      }
      NFA right = std::move(nfaStack.top());
      nfaStack.pop();
      NFA left = std::move(nfaStack.top());
      nfaStack.pop();
      nfaStack.push(NFA::concatenate(std::move(left), std::move(right)));
    } else if (c == '|') {
      if (nfaStack.size() < 2) {
        throw CompilerException(
            "Regex Error: Stack underflow for union operator '|'");
      }
      NFA right = std::move(nfaStack.top());
      nfaStack.pop();
      NFA left = std::move(nfaStack.top());
      nfaStack.pop();
      nfaStack.push(
          NFA::unionNFA(std::move(left), std::move(right), stateCounter));
    } else if (c == '*') {
      if (nfaStack.empty()) {
        throw CompilerException(
            "Regex Error: Stack underflow for Kleene star operator '*'");
      }
      NFA nfa = std::move(nfaStack.top());
      nfaStack.pop();
      nfaStack.push(NFA::kleeneStar(std::move(nfa), stateCounter));
    } else if (c == '+') {
      if (nfaStack.empty()) {
        throw CompilerException(
            "Regex Error: Stack underflow for one-or-more operator '+'");
      }
      NFA nfa = std::move(nfaStack.top());
      nfaStack.pop();
      nfaStack.push(NFA::oneOrMore(std::move(nfa), stateCounter));
    } else if (c == '?') {
      if (nfaStack.empty()) {
        throw CompilerException(
            "Regex Error: Stack underflow for zero-or-one operator '?'");
      }
      NFA nfa = std::move(nfaStack.top());
      nfaStack.pop();
      nfaStack.push(NFA::zeroOrOne(std::move(nfa), stateCounter));
    }
  }

  if (nfaStack.empty()) {
    throw CompilerException("Regex Error: Empty stack after parsing");
  }
  if (nfaStack.size() > 1) {
    throw CompilerException(
        "Regex Error: Invalid regex, multiple NFAs remaining on stack");
  }
  return std::move(nfaStack.top());
}
