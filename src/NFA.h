#ifndef NFA_H
#define NFA_H

#include "State.h"
#include <iostream>
#include <stack>
#include <string>
#include <vector>

class NFA {
public:
  State *startState;
  State *acceptState;
  std::vector<State *> allStates; // Keep track to delete later

  NFA() : startState(nullptr), acceptState(nullptr) {}

  // Destructor
  ~NFA() { destroy(); }

  // Move Constructor
  NFA(NFA &&other) noexcept
      : startState(other.startState), acceptState(other.acceptState),
        allStates(std::move(other.allStates)) {
    other.startState = nullptr;
    other.acceptState = nullptr;
  }

  // Move Assignment
  NFA &operator=(NFA &&other) noexcept {
    if (this != &other) {
      destroy();
      startState = other.startState;
      acceptState = other.acceptState;
      allStates = std::move(other.allStates);

      other.startState = nullptr;
      other.acceptState = nullptr;
    }
    return *this;
  }

  // Delete Copy
  NFA(const NFA &) = delete;
  NFA &operator=(const NFA &) = delete;

  // Create a basic NFA for a single character
  static NFA fromChar(char c, int &stateCounter);

  // Thompson's Construction Operations
  static NFA concatenate(NFA first, NFA second);
  static NFA unionNFA(NFA first, NFA second, int &stateCounter);
  static NFA kleeneStar(NFA nfa, int &stateCounter);
  static NFA oneOrMore(NFA nfa, int &stateCounter); // +
  static NFA zeroOrOne(NFA nfa, int &stateCounter); // ?

  // Helper to clean up memory
  void destroy() {
    for (State *s : allStates) {
      delete s;
    }
    allStates.clear();
  }

  void print();
  void toDot(std::string filename, std::string label);
};

#endif // NFA_H
