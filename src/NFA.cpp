#include "NFA.h"
#include <fstream>

NFA NFA::fromChar(char c, int &stateCounter) {
  NFA nfa;
  nfa.startState = new State(stateCounter++);
  nfa.acceptState = new State(stateCounter++, true);
  nfa.startState->addTransition(c, nfa.acceptState);

  nfa.allStates.push_back(nfa.startState);
  nfa.allStates.push_back(nfa.acceptState);
  return nfa;
}

NFA NFA::concatenate(NFA first, NFA second) {
  NFA nfa;
  nfa.startState = first.startState;
  nfa.acceptState = second.acceptState;

  // Connect first's accept state to second's start state with Epsilon
  first.acceptState->isAccepting = false;
  first.acceptState->addTransition(EPSILON, second.startState);

  // Merge state lists
  // We take ownership of states from first and second
  nfa.allStates = std::move(first.allStates);
  nfa.allStates.insert(nfa.allStates.end(), second.allStates.begin(),
                       second.allStates.end());

  // Clear second's states so they don't get deleted when second dies
  second.allStates.clear();

  return nfa;
}

NFA NFA::unionNFA(NFA first, NFA second, int &stateCounter) {
  NFA nfa;
  nfa.startState = new State(stateCounter++);
  nfa.acceptState = new State(stateCounter++, true);

  // Epsilon transitions from new start to both NFAs
  nfa.startState->addTransition(EPSILON, first.startState);
  nfa.startState->addTransition(EPSILON, second.startState);

  // Epsilon transitions from both NFAs to new accept
  first.acceptState->isAccepting = false;
  first.acceptState->addTransition(EPSILON, nfa.acceptState);

  second.acceptState->isAccepting = false;
  second.acceptState->addTransition(EPSILON, nfa.acceptState);

  // Collect all states
  nfa.allStates.push_back(nfa.startState);
  nfa.allStates.push_back(nfa.acceptState);

  nfa.allStates.insert(nfa.allStates.end(), first.allStates.begin(),
                       first.allStates.end());
  first.allStates.clear();

  nfa.allStates.insert(nfa.allStates.end(), second.allStates.begin(),
                       second.allStates.end());
  second.allStates.clear();

  return nfa;
}

NFA NFA::kleeneStar(NFA nfa, int &stateCounter) {
  NFA newNFA;
  newNFA.startState = new State(stateCounter++);
  newNFA.acceptState = new State(stateCounter++, true);

  // Epsilon from new start to old start
  newNFA.startState->addTransition(EPSILON, nfa.startState);

  // Epsilon from new start to new accept (for 0 occurrences)
  newNFA.startState->addTransition(EPSILON, newNFA.acceptState);

  // Epsilon from old accept to old start (loop)
  nfa.acceptState->isAccepting = false;
  nfa.acceptState->addTransition(EPSILON, nfa.startState);

  // Epsilon from old accept to new accept
  nfa.acceptState->addTransition(EPSILON, newNFA.acceptState);

  // Collect states
  newNFA.allStates.push_back(newNFA.startState);
  newNFA.allStates.push_back(newNFA.acceptState);
  newNFA.allStates.insert(newNFA.allStates.end(), nfa.allStates.begin(),
                          nfa.allStates.end());
  nfa.allStates.clear();

  return newNFA;
}

NFA NFA::oneOrMore(NFA nfa, int &stateCounter) {
  // a+ is aa*
  // But to avoid cloning 'a', we can just structure it similarly to Kleene star
  // but without the 0-occurrence bypass. Actually, standard construction: New
  // Start -> Old Start Old Accept -> New Accept Old Accept -> Old Start (Loop)

  NFA newNFA;
  newNFA.startState = new State(stateCounter++);
  newNFA.acceptState = new State(stateCounter++, true);

  newNFA.startState->addTransition(EPSILON, nfa.startState);

  nfa.acceptState->isAccepting = false;
  nfa.acceptState->addTransition(EPSILON, newNFA.acceptState);
  nfa.acceptState->addTransition(EPSILON, nfa.startState); // Loop back

  newNFA.allStates.push_back(newNFA.startState);
  newNFA.allStates.push_back(newNFA.acceptState);
  newNFA.allStates.insert(newNFA.allStates.end(), nfa.allStates.begin(),
                          nfa.allStates.end());
  nfa.allStates.clear();

  return newNFA;
}

NFA NFA::zeroOrOne(NFA nfa, int &stateCounter) {
  // a?
  // New Start -> Old Start
  // New Start -> New Accept (Skip)
  // Old Accept -> New Accept

  NFA newNFA;
  newNFA.startState = new State(stateCounter++);
  newNFA.acceptState = new State(stateCounter++, true);

  newNFA.startState->addTransition(EPSILON, nfa.startState);
  newNFA.startState->addTransition(EPSILON, newNFA.acceptState);

  nfa.acceptState->isAccepting = false;
  nfa.acceptState->addTransition(EPSILON, newNFA.acceptState);

  newNFA.allStates.push_back(newNFA.startState);
  newNFA.allStates.push_back(newNFA.acceptState);
  newNFA.allStates.insert(newNFA.allStates.end(), nfa.allStates.begin(),
                          nfa.allStates.end());
  nfa.allStates.clear();

  return newNFA;
}

void NFA::print() {
  std::cout << "Start State: " << startState->id << std::endl;
  std::cout << "Accept State: " << acceptState->id << std::endl;
  std::cout << "Transitions:" << std::endl;
  for (State *s : allStates) {
    for (auto const &[key, val] : s->transitions) {
      std::string input = (key == EPSILON) ? "EPSILON" : std::string(1, key);
      std::cout << "  " << s->id << " --(" << input << ")--> " << val->id
                << std::endl;
    }
    if (s->isAccepting) {
      std::cout << "  " << s->id << " [ACCEPT]" << std::endl;
    }
  }
}

void NFA::toDot(std::string filename, std::string label) {
  std::ofstream out(filename);
  out << "digraph NFA {" << std::endl;
  out << "  rankdir=LR;" << std::endl;
  out << "  label=\"" << label << "\";" << std::endl;
  out << "  node [shape=circle];" << std::endl;
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
