#include "DFA.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <queue>

std::set<State *> DFA::epsilonClosure(std::set<State *> states) {
  std::set<State *> closure = states;
  std::stack<State *> stack;
  for (State *s : states)
    stack.push(s);

  while (!stack.empty()) {
    State *current = stack.top();
    stack.pop();

    auto range = current->transitions.equal_range(EPSILON);
    for (auto it = range.first; it != range.second; ++it) {
      State *next = it->second;
      if (closure.find(next) == closure.end()) {
        closure.insert(next);
        stack.push(next);
      }
    }
  }
  return closure;
}

std::set<State *> DFA::move(std::set<State *> states, char input) {
  std::set<State *> result;
  for (State *s : states) {
    auto range = s->transitions.equal_range(input);
    for (auto it = range.first; it != range.second; ++it) {
      result.insert(it->second);
    }
  }
  return result;
}

DFA DFA::fromNFA(NFA nfa) {
  DFA dfa;
  int stateCounter = 0;

  std::set<State *> startSet;
  startSet.insert(nfa.startState);
  std::set<State *> initialClosure = epsilonClosure(startSet);

  dfa.startState = new State(stateCounter++);
  // Check if start state is accepting
  for (State *s : initialClosure) {
    if (s->isAccepting) {
      dfa.startState->isAccepting = true;
      break;
    }
  }
  dfa.allStates.push_back(dfa.startState);

  std::map<std::set<State *>, State *> dfaStates;
  dfaStates[initialClosure] = dfa.startState;

  std::queue<std::set<State *>> worklist;
  worklist.push(initialClosure);

  while (!worklist.empty()) {
    std::set<State *> currentSet = worklist.front();
    worklist.pop();
    State *currentDfaState = dfaStates[currentSet];

    // Find all possible inputs from this set of states
    std::set<char> inputs;
    for (State *s : currentSet) {
      for (auto const &[key, val] : s->transitions) {
        if (key != EPSILON) {
          inputs.insert(key);
        }
      }
    }

    for (char input : inputs) {
      std::set<State *> nextSet = epsilonClosure(move(currentSet, input));

      if (nextSet.empty())
        continue;

      if (dfaStates.find(nextSet) == dfaStates.end()) {
        State *newState = new State(stateCounter++);
        for (State *s : nextSet) {
          if (s->isAccepting) {
            newState->isAccepting = true;
            break;
          }
        }
        dfaStates[nextSet] = newState;
        dfa.allStates.push_back(newState);
        worklist.push(nextSet);
      }

      currentDfaState->addTransition(input, dfaStates[nextSet]);
    }
  }

  return dfa;
}

void DFA::print() {
  std::cout << "DFA Start State: " << startState->id << std::endl;
  std::cout << "Transitions:" << std::endl;
  for (State *s : allStates) {
    for (auto const &[key, val] : s->transitions) {
      std::cout << "  " << s->id << " --(" << key << ")--> " << val->id
                << std::endl;
    }
    if (s->isAccepting) {
      std::cout << "  " << s->id << " [ACCEPT]" << std::endl;
    }
  }
}

bool DFA::simulate(std::string input, bool debug) {
  State *current = startState;
  int step = 0;

  if (debug) {
    std::cout << "\n[Trace] DFA Simulation Trace:\n";
  }

  for (size_t i = 0; i < input.length(); ++i) {
    char c = input[i];

    // Visualizer Format: Step N: Read 'c' at position I
    if (debug) {
      step++;
      std::cout << "Step " << step << ": Read '" << c << "' at position " << i
                << "\n";
    }

    auto range = current->transitions.equal_range(c);
    if (range.first == range.second) {
      if (debug)
        std::cout << "Action: No Transition (REJECT)\nStack: [q" << current->id
                  << "]\n";
      return false;
    }

    State *nextState = range.first->second;

    // Visualizer Format reuse:
    // Action -> "Transition qX -> qY"
    // Stack -> "[qY]" (Current State)
    if (debug) {
      std::cout << "Action: Transition q" << current->id << " -> q"
                << nextState->id << "\n";
      std::cout << "Stack: [q" << nextState->id << "]\n";
    }

    current = nextState;
  }

  bool result = current->isAccepting;
  if (debug) {
    std::cout << "Step " << step + 1 << ": End of Input\n";
    std::cout << "Action: " << (result ? "ACCEPT" : "REJECT") << "\n";
    std::cout << "Stack: [q" << current->id << "]\n";
  }

  return result;
}

void DFA::minimize() {
  if (allStates.empty())
    return;

  // 0. Collect all inputs
  std::set<char> inputs;
  for (State *s : allStates) {
    for (auto const &[key, val] : s->transitions) {
      inputs.insert(key);
    }
  }

  // 1. Initial Partition: Accepting and Non-Accepting
  std::vector<std::set<State *>> partitions;
  std::set<State *> accepting, nonAccepting;
  for (State *s : allStates) {
    if (s->isAccepting)
      accepting.insert(s);
    else
      nonAccepting.insert(s);
  }
  if (!accepting.empty())
    partitions.push_back(accepting);
  if (!nonAccepting.empty())
    partitions.push_back(nonAccepting);

  // 2. Refine Partitions
  bool changed = true;
  while (changed) {
    changed = false;
    std::vector<std::set<State *>> newPartitions;

    for (auto &group : partitions) {
      if (group.size() <= 1) {
        newPartitions.push_back(group);
        continue;
      }

      std::map<std::vector<int>, std::set<State *>> splitGroups;

      for (State *s : group) {
        std::vector<int> signature;
        for (char c : inputs) {
          int targetGroupIdx = -1;
          // Find where s goes on input c
          auto it = s->transitions.find(c);
          if (it != s->transitions.end()) {
            State *target = it->second;
            // Find which group target belongs to
            for (size_t i = 0; i < partitions.size(); ++i) {
              if (partitions[i].count(target)) {
                targetGroupIdx = i;
                break;
              }
            }
          }
          signature.push_back(targetGroupIdx);
        }
        splitGroups[signature].insert(s);
      }

      if (splitGroups.size() > 1) {
        changed = true;
      }
      for (auto const &[sig, subgroup] : splitGroups) {
        newPartitions.push_back(subgroup);
      }
    }
    partitions = newPartitions;
  }

  // 3. Reconstruct DFA
  // Create new states for each partition
  std::vector<State *> newStates;
  std::map<int, State *> partitionToState; // partition index -> new State

  for (size_t i = 0; i < partitions.size(); ++i) {
    State *newState = new State(i);
    // If any state in partition is accepting, new state is accepting
    for (State *s : partitions[i]) {
      if (s->isAccepting) {
        newState->isAccepting = true;
        break;
      }
    }
    newStates.push_back(newState);
    partitionToState[i] = newState;
  }

  // Set Start State
  for (size_t i = 0; i < partitions.size(); ++i) {
    if (partitions[i].count(startState)) {
      startState = partitionToState[i];
      break;
    }
  }

  // Add Transitions
  for (size_t i = 0; i < partitions.size(); ++i) {
    State *representative = *partitions[i].begin();
    State *source = partitionToState[i];

    for (char c : inputs) {
      auto it = representative->transitions.find(c);
      if (it != representative->transitions.end()) {
        State *oldTarget = it->second;
        // Find which partition oldTarget belongs to
        int targetPartitionIdx = -1;
        for (size_t j = 0; j < partitions.size(); ++j) {
          if (partitions[j].count(oldTarget)) {
            targetPartitionIdx = j;
            break;
          }
        }
        if (targetPartitionIdx != -1) {
          source->addTransition(c, partitionToState[targetPartitionIdx]);
        }
      }
    }
  }

  // Cleanup old states
  for (State *s : allStates) {
    delete s;
  }
  allStates = newStates;
}

void DFA::toDot(std::string filename, std::string label) {
  std::ofstream out(filename);
  out << "digraph DFA {" << std::endl;
  out << "  rankdir=LR;" << std::endl;
  out << "  label=\"" << label << "\";" << std::endl;
  out << "  node [shape=circle];" << std::endl;

  if (startState) {
    out << "  start [shape=point];" << std::endl;
    out << "  start -> " << startState->id << ";" << std::endl;
  }

  for (State *s : allStates) {
    if (s->isAccepting) {
      out << "  " << s->id << " [shape=doublecircle];" << std::endl;
    }
    for (auto const &[key, val] : s->transitions) {
      std::string label = (key == EPSILON) ? "ε" : std::string(1, key);
      out << "  " << s->id << " -> " << val->id << " [label=\"" << label
          << "\"];" << std::endl;
    }
  }
  out << "}" << std::endl;
  out.close();
}
