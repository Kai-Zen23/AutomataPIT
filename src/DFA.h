#ifndef DFA_H
#define DFA_H

#include "NFA.h"
#include <map>
#include <set>
#include <vector>

class DFA {
public:
  State *startState;
  std::vector<State *> allStates;

  DFA() : startState(nullptr) {}

  // Destructor
  ~DFA() { destroy(); }

  // Move Constructor
  DFA(DFA &&other) noexcept
      : startState(other.startState), allStates(std::move(other.allStates)) {
    other.startState = nullptr;
  }

  // Move Assignment
  DFA &operator=(DFA &&other) noexcept {
    if (this != &other) {
      destroy();
      startState = other.startState;
      allStates = std::move(other.allStates);

      other.startState = nullptr;
    }
    return *this;
  }

  // Delete Copy
  DFA(const DFA &) = delete;
  DFA &operator=(const DFA &) = delete;

  static DFA fromNFA(NFA nfa);

  void minimize();
  void print();
  void toDot(std::string filename, std::string label);
  void destroy() {
    for (State *s : allStates) {
      delete s;
    }
    allStates.clear();
  }

private:
  static std::set<State *> epsilonClosure(std::set<State *> states);
  static std::set<State *> move(std::set<State *> states, char input);
};

#endif // DFA_H
