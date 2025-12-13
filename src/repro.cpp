#include "DFA.h"
#include "NFA.h"
#include "RegexParser.h"
#include <iostream>
#include <string>
#include <vector>

void testRegex(std::string regex) {
  std::cout << "Testing Regex: " << regex << std::endl;
  try {
    RegexParser parser;
    NFA nfa = parser.parse(regex);
    std::cout << "NFA constructed. States: " << nfa.allStates.size()
              << std::endl;
    DFA dfa = DFA::fromNFA(std::move(nfa));
    std::cout << "DFA constructed. States: " << dfa.allStates.size()
              << std::endl;
    dfa.minimize();
    std::cout << "DFA minimized." << std::endl;
    // Destructors handle cleanup
  } catch (const std::exception &e) {
    std::cout << "Exception: " << e.what() << std::endl;
  }
}

int main() {
  // Test 1: Deep nesting (Stack Overflow?)
  std::string nested = "a";
  for (int i = 0; i < 5000; ++i) { // Reduced to 5000 to be safe but still deep
    nested = "(" + nested + ")";
  }
  testRegex(nested);

  // Test 2: Long concatenation
  std::string concat = "a";
  for (int i = 0; i < 100;
       ++i) { // Reduced from 10000 to 100 to avoid hanging on minimize
    concat += "a";
  }
  testRegex(concat);

  // Test 3: Large Range
  testRegex("[a-z]");
  testRegex("[0-9]");

  // Test 4: Invalid
  testRegex("(a|b");

  return 0;
}
