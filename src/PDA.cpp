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

  while (!worklist.empty()) {
    SimulationState current = worklist.back();
    worklist.pop_back();

    if (debug) {
      std::cout << "State: " << current.currentState
                << ", InputIdx: " << current.inputIndex << ", Stack: ";
      for (const auto &c : current.stack)
        std::cout << c << " ";
      std::cout << std::endl;
    }

    if (current.inputIndex == inputTokens.size() &&
        acceptStates.count(current.currentState)) {
      if (debug)
        std::cout << "ACCEPTED!" << std::endl;
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
        for (auto it = res.pushSymbols.rbegin(); it != res.pushSymbols.rend();
             ++it) {
          nextState.stack.push_back(*it);
        }
        worklist.push_back(nextState);
      }
    }
  }

  if (debug)
    std::cout << "REJECTED" << std::endl;
  return false;
}
