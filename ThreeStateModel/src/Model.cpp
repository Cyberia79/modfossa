/*
 * Model.cpp
 *
 *  Created on: Jun 13, 2012
 *      Author: gareth
 */

#include <vector>

#include <ModFossa/Model.h>

namespace ModFossa {

Model::Model() {
}

Model::~Model() {
}

Model* Model::getInstance() {
    static Model instance_;
    return &instance_;
}

void Model::addState(StateName name) {
    State state;
    state.name = name;
    state.is_initial = false;

    states.push_back(state);
}

void Model::setConducting(StateName name) {
    State* state = getStateByName(name);
    state->is_conducting = true;
}

void Model::connect(StateName from, StateName to, double rate) {
    State* from_state = getStateByName(from);
    State* to_state = getStateByName(to);

    Transition outgoing;
    outgoing.rate = rate;
    outgoing.source = from; // this shouldn't be used anywhere for outgoing transitions
    from_state->out_transitions.push_back(outgoing);

    Transition incoming;
    incoming.rate = rate;
    incoming.source = from; // this IS used for calculating incoming transitions
    to_state->in_transitions.push_back(incoming);
}

void Model::setInitialState(StateName name) {
    State* state = getStateByName(name);
    state->is_initial = true;
}

void Model::setIntegrationWindow(std::vector<double> times) {
    this->times = times;
}

boost::numeric::ublas::matrix<double> Model::run() {
    // Create voltage protocol. The values must be set already.
    voltage_protocol.generateVoltageProtocol();

    // Test voltage protocol
    std::vector<VoltageClamp>::iterator it;
    for (it = voltage_protocol.voltage_clamps.begin();
            it != voltage_protocol.voltage_clamps.end(); ++it) {
        std::cout << it->getVoltageAtTime(0) << " ";
        std::cout << it->getVoltageAtTime(0.01) << " ";
        std::cout << it->getVoltageAtTime(0.1) << " ";
        std::cout << it->getVoltageAtTime(0.101) << " ";
        std::cout << it->getVoltageAtTime(1.1) << " ";
        std::cout << it->getVoltageAtTime(1.11) << " ";
        std::cout << it->getVoltageAtTime(2.1) << " ";
        std::cout << it->getVoltageAtTime(2.2) << "\n";
    }

    boost::numeric::ublas::matrix<double> result;
    std::vector<double> initial_conditions;

    for (unsigned int i = 0; i < states.size(); ++i) {
        if (states[i].is_initial == 1) {
            initial_conditions.push_back(1);
        } else {
            initial_conditions.push_back(0);
        }
    }

    SundialsCpp open_solver;
    open_solver.solve(channelProb, times, initial_conditions, result);

    return result;
}

void Model::setVstep(std::vector<double> voltages) {
    voltage_protocol.setVstep(voltages);
}

void Model::setVholdStart(double v_hold_start) {
    voltage_protocol.setVholdStart(v_hold_start);
}

void Model::setVholdFinish(double v_hold_finish) {
    voltage_protocol.setVholdFinish(v_hold_finish);
}

void Model::setEventTimes(double start, double step, double finish,
        double end) {
    voltage_protocol.setEventTimes(start, step, finish, end);
}

int Model::numStates() const {
    return states.size();
}

void Model::setState(const N_Vector& state) {
    for (unsigned int i = 0; i < states.size(); ++i) {
        states[i].current_probability = NV_Ith_S(state, i);
    }
}

double Model::inProb(int index) {
    double in_prob = 0;
    State state = states[index];

    for (unsigned int i = 0; i < state.in_transitions.size(); ++i) {
        Transition incoming_transition = state.in_transitions[i];
        double incoming_rate = incoming_transition.rate;
        State* incoming_state = getStateByName(incoming_transition.source);

        in_prob += incoming_rate * incoming_state->current_probability;
    }

    return in_prob;
}

double Model::outProb(int index) {
    double out_prob = 0;
    State state = states[index];

    for (unsigned int i = 0; i < state.out_transitions.size(); ++i) {
        Transition outgoing_transition = state.out_transitions[i];
        double outgoing_rate = outgoing_transition.rate;
        out_prob += outgoing_rate * state.current_probability;
    }

    return out_prob;
}

State* Model::getStateByName(StateName name) {
    for (unsigned int i = 0; i < states.size(); ++i) {
        if (states[i].name == name) {
            return &(states[i]);
        }
    }
    return NULL;
}

// Static RHS function used by Sundials. Calculates the value of the
// right hand side of the differential equation at the next timestep, t.
static int channelProb(realtype t, N_Vector y_vec, N_Vector ydot,
        void *f_data) {

    Model* model = Model::getInstance();
    // Set current state. Theses are state
    // probabilities of the ion channel model.
    model->setState(y_vec);

    // Set the derivative of each state probability.
    // This is equal to the total probability of transitioning
    // into the state, minus the probability of transitioning out.
    for (int i = 0; i < model->numStates(); ++i) {
        double p_dot_i = model->inProb(i) - model->outProb(i);
        NV_Ith_S(ydot, i) = p_dot_i;
    }

    return 0;
}

} /* namespace ModFossa */
