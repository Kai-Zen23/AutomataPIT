#ifndef STATE_H
#define STATE_H

#include <vector>
#include <map>
#include <string>

// Epsilon transition is represented by a special character, e.g., '\0' or a specific constant.
const char EPSILON = '\0';

struct State {
    int id;
    bool isAccepting;
    // Transitions: input char -> list of next state IDs
    std::multimap<char, State*> transitions;

    State(int id, bool isAccepting = false) : id(id), isAccepting(isAccepting) {}

    void addTransition(char input, State* nextState) {
        transitions.insert({input, nextState});
    }
};

#endif // STATE_H
