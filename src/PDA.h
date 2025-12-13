#ifndef PDA_H
#define PDA_H

#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <vector>


// Represents a transition key: (Current State, Input Symbol, Stack Top)
struct PDATransitionKey {
  int currentState;
  std::string input;    // Changed from char to string for Token Types
  std::string stackTop; // Changed from char to string for Stack Symbols

  bool operator<(const PDATransitionKey &other) const {
    if (currentState != other.currentState)
      return currentState < other.currentState;
    if (input != other.input)
      return input < other.input;
    return stackTop < other.stackTop;
  }
};

// Represents a transition result: (Next State, Symbols to Push)
struct PDATransitionResult {
  int nextState;
  std::vector<std::string> pushSymbols; // Changed from char to string
};

class PDA {
public:
  int startState;
  std::set<int> acceptStates;
  std::string startStackSymbol;

  // Transitions map
  std::map<PDATransitionKey, std::vector<PDATransitionResult>> transitions;

  PDA(int start, std::string stackStart)
      : startState(start), startStackSymbol(stackStart) {}

  void addTransition(int state, std::string input, std::string stackTop,
                     int nextState, std::vector<std::string> pushSymbols);
  void addAcceptState(int state);

  // Simulate the PDA on an input string (vector of tokens)
  bool simulate(std::vector<std::string> inputTokens, bool debug = false);
};

#endif // PDA_H
