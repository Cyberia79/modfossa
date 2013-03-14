/* 
 * File:   TransitionMatrix.cpp
 * Author: gareth
 * 
 * Created on March 11, 2013, 5:01 PM
 */

#include <ModelDefinition/TransitionMatrix.h>
#include <ModelDefinition/HelperFunctions.h>

#include <cassert>

namespace ModelDefinition {

    TransitionMatrix::TransitionMatrix(const MarkovModel& markov_model) {
        create(markov_model);
    }

    TransitionMatrix::~TransitionMatrix() {
    }

    double TransitionMatrix::calculateTotalRate(
        const vector<Transition::SharedPointer>& transitions,
            const StateOfTheWorld::SharedPointer state_of_the_world) const {
        double total_rate = 0;
        
        vector<Transition::SharedPointer>::const_iterator it;
        for(it = transitions.begin(); it != transitions.end(); ++it) {
            
            double rate = (*it)->rate_constant->getRate(state_of_the_world);
            
            // Make sign negative if outgoing connection
            if((*it)->sign == false) { 
                rate *= -1;
            }
            
            total_rate += rate;
        }
        
        return total_rate;
    }
    
    void TransitionMatrix::update(
        StateOfTheWorld::SharedPointer state_of_the_world) {
        assert(transition_matrix.n_rows == transitions_3d.size());
        assert(transition_matrix.n_cols == transitions_3d[0].size());
        
        for(int i = 0; i < transition_matrix.n_rows; ++i) {
            for(int j = 0; j < transition_matrix.n_cols; ++j){
                transition_matrix(i, j) = calculateTotalRate(
                        transitions_3d[i][j], state_of_the_world);
            }
        }
    }

    void TransitionMatrix::create(const MarkovModel& markov_model) {

        if (!markov_model.isValid()) {
            throw std::runtime_error(
                    "Cannot create TransitionMatrix with unvalidated MarkovModel");
        }

        // Create the intermediate 3d transitions matrix of size NxN, where
        // N is the number of states.
        const int matrix_size = markov_model.map_of_states.size();
        transitions_3d.resize(matrix_size);
        for (int i = 0; i < matrix_size; ++i) {
            transitions_3d[i].resize(matrix_size);
        }

        // Iterate through our connections and add two Transition entries
        // to the intermediate 3d transitions matrix for each connection in the
        // model. 
        MarkovModel::ConnectionsVector::const_iterator it;
        for (it = markov_model.connections.begin();
                it != markov_model.connections.end(); ++it) {

            string from_state_name = (*it)->from_state;
            string to_state_name = (*it)->to_state;
            string rate_name = (*it)->rate_name;

            State::SharedPointer from_state =
                    findState(from_state_name, markov_model.map_of_states);
            State::SharedPointer to_state =
                    findState(to_state_name, markov_model.map_of_states);
            RateConstantBase::SharedPointer rate =
                    findRate(rate_name, markov_model.map_of_rates);

            // We need the indices that MarkovModel assigned to each state.
            // These will be the indices to the matrix.
            int outgoing_index = from_state->getIndex();
            int incoming_index = to_state->getIndex();

            // Add the transition to the appropriate position in the matrix 
            // with a negative sign
            transitions_3d[outgoing_index][outgoing_index].push_back(
                    Transition::SharedPointer(new Transition(rate, false)));

            // Add the reversed transition to the matrix with a positive sign
            transitions_3d[incoming_index][outgoing_index].push_back(
                    Transition::SharedPointer(new Transition(rate, true)));
            
            // Finally, resize our 'actual' 
            // transition matrix of doubles that will be used by the ODE solver.
            transition_matrix.resize(matrix_size, matrix_size);            
            //update();
        }
    }

    Matrix TransitionMatrix::get() const {
        return transition_matrix;
    }
}
