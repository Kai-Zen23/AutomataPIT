#include "PDA.h"

// Special constant for Epsilon in string form
const std::string EPSILON_STR = "EPSILON";

void PDA::addTransition(int state, std::string input, std::string stackTop,
                        int nextState, std::vector<std::string> pushSymbols) {
  PDATransitionKey key = {state, input, stackTop};
  PDATransitionResult result = {nextState, pushSymbols};
  transitions[key].push_back(result);
}

void PDA::addAcceptState(int state) { acceptStates.insert(state); }

// Helper struct for DFS state
struct SimulationState {
  int currentState;
  size_t inputIndex;
  std::vector<std::string> stack;
};

bool PDA::simulate(std::vector<std::string> inputTokens, bool debug) {
  std::vector<SimulationState> worklist;

  // Initial state
  std::vector<std::string> initialStack;
  initialStack.push_back(startStackSymbol);
  worklist.push_back({startState, 0, initialStack});

  if (debug) {
    std::cout << "Starting PDA Simulation on input tokens: ";
    for (const auto &t : inputTokens)
      std::cout << t << " ";
    std::cout << std::endl;
  }

  int stepCount = 0;

  while (!worklist.empty()) {
    SimulationState current = worklist.back();
    worklist.pop_back();

    if (debug) {
      stepCount++;
      std::string inputChar = (current.inputIndex < inputTokens.size())
                                  ? inputTokens[current.inputIndex]
                                  : "EPSILON";
      std::string pos = std::to_string(current.inputIndex);
      std::cout << "Step " << stepCount << ": Read '" << inputChar
                << "' at position " << pos << "\n";
    }

    if (current.inputIndex == inputTokens.size() &&
        acceptStates.count(current.currentState)) {
      if (debug) {
        std::cout << "  Action: ACCEPT\n";
        std::cout << "\n[5] Result:\n[ACCEPT] String is valid context-free "
                     "language string\n";
      }
      return true;
    }

    std::string nextInput = (current.inputIndex < inputTokens.size())
                                ? inputTokens[current.inputIndex]
                                : EPSILON_STR;

    if (current.stack.empty())
      continue;

    std::string stackTop = current.stack.back();

    // 1. Try consuming input
    if (current.inputIndex < inputTokens.size()) {
      PDATransitionKey key = {current.currentState, nextInput, stackTop};
      if (transitions.count(key)) {
        for (auto &res : transitions[key]) {
          SimulationState nextState = current;
          nextState.currentState = res.nextState;
          nextState.inputIndex++;
          nextState.stack.pop_back(); // Pop

          std::string actionDesc = "POP '" + stackTop + "'";
          if (!res.pushSymbols.empty()) {
            actionDesc = "PUSH ";
            for (const auto &s : res.pushSymbols)
              actionDesc += "'" + s + "' ";
            actionDesc += "(replaced '" + stackTop + "')";
          }

          if (debug) {
            std::cout << "  Action: " << actionDesc << "\n";
            std::cout << "  Stack: [";
            for (size_t i = 0; i < nextState.stack.size(); ++i) {
              std::cout << nextState.stack[i]
                        << (i < nextState.stack.size() - 1 ? ", " : "");
            }
            std::cout << "]\n\n";
          }

          // Push new symbols (reverse order)
          for (auto it = res.pushSymbols.rbegin(); it != res.pushSymbols.rend();
               ++it) {
            nextState.stack.push_back(*it);
          }
          worklist.push_back(nextState);
        }
      }
    }

    // 2. Try Epsilon transition
    PDATransitionKey epsKey = {current.currentState, EPSILON_STR, stackTop};
    if (transitions.count(epsKey)) {
      for (auto &res : transitions[epsKey]) {
        SimulationState nextState = current;
        nextState.currentState = res.nextState;
        // Input index doesn't change
        nextState.stack.pop_back();

        std::string actionDesc = "POP '" + stackTop + "'";
        if (!res.pushSymbols.empty()) {
          actionDesc = "EXPAND '" + stackTop + "' -> ";
          for (const auto &s : res.pushSymbols)
            actionDesc += s + " ";
        }

        if (debug) {
          std::cout << "  Action: " << actionDesc << "\n";
          std::cout << "  Stack: [";
          for (size_t i = 0; i < nextState.stack.size(); ++i) {
            std::cout << nextState.stack[i]
                      << (i < nextState.stack.size() - 1 ? ", " : "");
          }
          std::cout << "]\n\n";
        }

        for (auto it = res.pushSymbols.rbegin(); it != res.pushSymbols.rend();
             ++it) {
          nextState.stack.push_back(*it);
        }
        worklist.push_back(nextState);
      }
    }
  }

  if (debug)
    std::cout << "\n[5] Result:\n[REJECT] String is invalid\n";
  return false;
}

void PDA::toDot(std::string filename) {
  std::ofstream out(filename);
  out << "digraph PDA {" << std::endl;
  out << "  rankdir=LR;" << std::endl;
  out << "  label=\"Pushdown Automaton\";" << std::endl;
  out << "  node [shape=circle];" << std::endl;

  // Start state pointer
  out << "  start [shape=point];" << std::endl;
  out << "  start -> " << startState << ";" << std::endl;

  // Accessing private acceptStates is allowed here since we are in the class
  for (int s : acceptStates) {
    out << "  " << s << " [shape=doublecircle];" << std::endl;
  }

  // Transitions
  // Map Key: {currentState, input, stackTop}
  // Map Value: vector of {nextState, pushSymbols}
  for (auto const &[key, results] : transitions) {
    for (auto const &res : results) {
      std::string label = "";

      // Input
      label += (key.input == EPSILON_STR) ? "ε" : key.input;
      label += ", ";

      // Pop
      label += key.stackTop;
      label += " -> ";

      // Push
      if (res.pushSymbols.empty()) {
        label += "ε";
      } else {
        for (size_t i = 0; i < res.pushSymbols.size(); ++i) {
          label += res.pushSymbols[i];
          if (i < res.pushSymbols.size() - 1)
            label += " ";
        }
      }

      out << "  " << key.currentState << " -> " << res.nextState << " [label=\""
          << label << "\"];" << std::endl;
    }
  }

  out << "}" << std::endl;
  out.close();
}
