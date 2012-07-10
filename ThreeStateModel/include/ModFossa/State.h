/*
 * State.h
 *
 *  Created on: Jul 10, 2012
 *      Author: gareth
 */

#include <ModFossa/Transition.h>

class State {
public:
    StateName name;
    std::vector<Transition> in_transitions;
    std::vector<Transition> out_transitions;
    bool is_initial;
    bool is_conducting;
    double current_probability;
};