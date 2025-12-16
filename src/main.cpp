#include "CompilerFrontend.h"
#include "DFA.h"
#include "RegexParser.h"
#include <iostream>
#include <string>

int main() {
  std::string modeLine;
  if (!std::getline(std::cin, modeLine) || modeLine.empty()) {
    std::cerr << "Error: No mode provided." << std::endl;
    return 1;
  }

  int mode = 0;
  try {
    mode = std::stoi(modeLine);
  } catch (...) {
    // Fallback if not an integer (e.g. running manually without mode)
    // Check if it's the calculator loop request
    mode = 3;
  }

  std::string inputLine;
  if (!std::getline(std::cin, inputLine)) {
    std::cerr << "Error: No input provided." << std::endl;
    return 1;
  }

  if (mode == 1) {
    // Regex / NFA / DFA
    std::cout << "Mode: Regex Analysis" << std::endl;
    std::cout << "Regex: " << inputLine << std::endl;

    try {
      RegexParser parser;
      NFA nfa = parser.parse(inputLine);

      std::cout << "[4] Thompson's NFA Construction:\n";
      std::cout << "  States created: " << nfa.allStates.size() << "\n";
      std::cout << "  Start state: q" << nfa.startState->id << "\n";
      std::cout << "  Final states: q" << nfa.acceptState->id << "\n\n";

      std::cout << "  Key Transitions:\n";
      int count = 0;
      for (auto const &[key, val] : nfa.startState->transitions) {
        std::string label = (key == EPSILON) ? "e" : std::string(1, key);
        std::cout << "    q" << nfa.startState->id << " --[" << label
                  << "]--> q" << val->id << "\n";
        if (++count >= 5)
          break;
      }
      std::cout << "\n";

      nfa.toDot("nfa.dot", "NFA for " + inputLine);

      DFA dfa = DFA::fromNFA(std::move(nfa));
      dfa.toDot("dfa.dot", "DFA for " + inputLine);

      dfa.minimize();
      dfa.toDot("min_dfa.dot", "Minimized DFA for " + inputLine);

      std::cout << "[5] NFA Simulation (Subset Construction):\n";
      std::cout << "  Testing strings against NFA...\n\n";

      std::string testString;
      if (std::getline(std::cin, testString) && !testString.empty()) {
        // Use user provided test string
        if (testString.back() == '\r')
          testString.pop_back(); // Handle Windows line endings
      } else {
        testString = "myVar"; // Fallback
      }

      // Run detailed trace for the test string
      std::cout << "\n[Detailed Animation Trace for '" << testString << "']\n";
      dfa.simulate(testString, true);
      std::cout << "[End Trace]\n\n";

      int testId = 1;
      for (const auto &s : testStrings) {
        bool result = dfa.simulate(s);
        std::cout << "  Test " << testId++ << ": \"" << s << "\"\n";
        std::cout << "    Result: "
                  << (result ? "[MATCH] Accepted" : "[NO MATCH] Rejected")
                  << "\n";
      }

      std::cout << "\nGraphs generated: nfa.dot, dfa.dot, min_dfa.dot\n";
      std::cout << "\n--- Summary ---\n";
      std::cout << "Regular Language: Recognized by finite automaton\n";
      std::cout << "Equivalence: Regex == NFA == DFA == Regular Grammar\n";

    } catch (const std::exception &e) {
      std::cerr << "Regex Error: " << e.what() << std::endl;
    }

  } else if (mode == 3) {
    // Calculator PDA
    // std::cout << "Mode: Calculator Simulation" << std::endl;
    CompilerFrontend compiler;
    compiler.run(inputLine);
  } else {
    std::cerr << "Unknown Mode: " << mode << std::endl;
  }

  return 0;
}
