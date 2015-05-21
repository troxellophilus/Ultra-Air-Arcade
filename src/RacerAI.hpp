/*
 * Class to define and implement a Racer AI component
 * Drew Troxell
 */

#ifndef RACER_AI_H
#define RACER_AI_H

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Entity.hpp"
#include "Collision.hpp"

// Track Target Positions
glm::vec3 track[10] = {glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(2, 0, 0),
			glm::vec3(3, 0, 0), glm::vec3(4, 0, 0), glm::vec3(5, 0, 0),
			glm::vec3(6, 0, 0), glm::vec3(7, 0, 0), glm::vec3(8, 0, 0),
			glm::vec3(9, 0, 0)};

class RacerAI {
private:
    // Race Status Vars
    int lap;
    int place;
    int track_idx;
    int next_idx;

public:
    // Constructors
    RacerAI();
    
    // Methods
    void update(Entity *agent);
};

// Constructors
RacerAI::RacerAI() {
    lap = 0;
    place = 0;
    track_idx = 0;
    next_idx = 1;
}

// Methods
void RacerAI::update(Entity *agent) {
    static unsigned int frames = 0;
    static int i = 0;

    agent->throttleUp();

    // Update track targets
    //  if (distance_to_target < test_radius) {
    //      track_idx = next_idx;
    // 	    next_idx++;
    // 	    if (next_idx > num_track_indices)
    // 	    	next_idx = 0;
    // 	}

    // Face the agent towards the target point
    //  glm::vec3 to_next = glm::normalize(track[next_idx] - agent.getPosition());
    //  glm::vec3 rot_axis = glm::cross(agent.getDirection(), to_next);
    //  glm::quat rotQ = glm::toQuat(rotM);
    //  agent->setTargetRotationQ(agent->getRotationQ() * rotQ);

    frames++;
}

// Getters

// Setters

#endif
