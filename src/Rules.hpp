/*
 * Class to define and implement the rules of the game
 * Drew Troxell
 */

#ifndef RULES_H
#define RULES_H

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Entity.hpp"

enum GameMode { RACE, TIME_TRIAL, FREE_FLY, DEBUG };
enum Racer { PLAYER, AI1, AI2, AI3, AI4, MAX_PLAYERS };

struct MapLoc {
    int x;
    int y;
};

class Rules {
private:
    // Mode
    GameMode mode; // mode of the camera
    
    // Players
    Entity *player; // Pointer to the player entity
    vector<Entity> *agents; // Pointer to the vector of opponents in main
    unsigned int lap[MAX_PLAYERS]; // Array of current lap # per racer

    // Map targets
    vector<struct MapLoc> targets;
    
public:
    // Constructors
    Rules();
    
    // Methods
    void update();

    // Getters
    
    // Setters
    void setAgents(vector<Entity> *a);
};

// Constructors
Rules::Rules() {
    mode = DEBUG; // init game mode
    
    agents = NULL; // set by main

    // initialize laps
    for (int i = 0; i < MAX_PLAYERS; i++)
        lap[i] = 0;
}

// Methods
void Rules::update() {
    static unsigned int frames = 0;
    static int i = 0;

    glm::vec3 target = glm::vec3(190, 35, 190);

    // check the racers' locations and update laps
    for (auto &agent : *agents) {
        glm::vec3 pos = agent.getPosition();

	if (target.x - pos.x || target.z - pos.z) {
            agent.turn(glm::max(target.x-pos.x, target.z-pos.z) / 20.f);
	}

	if (target.y - pos.y)
            agent.pitch(pos.y - target.y);

	agent.throttleUp();
    }

    frames++;
}

// Getters

// Setters
void Rules::setAgents(vector<Entity> *a) {
    agents = a;
}

#endif
