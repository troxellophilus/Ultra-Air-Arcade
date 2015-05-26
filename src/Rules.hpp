/*
 * Class to define and implement the rules of the game
 * Drew Troxell
 */

#ifndef RULES_H
#define RULES_H

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Entity.hpp"
#include "Collision.hpp"

class Rules {
private:
    enum GameState { SPLASH, CSEL, RACE, FREE_FLY, TIME_TRIAL, FINISH, LEADERBOARD };

    // Mode
    GameState state;
    
    // Players
    Entity *player; // Pointer to the player entity
    vector<Entity> *agents; // Pointer to the vector of opponents in main

    // state methods
    void splash();
    void characterSelect();
    void race();
    void freeFly();
    void timeTrial();
    void finish();
    void leaderboard();
    
public:
    // Constructors
    Rules();
    
    // Methods
    void update();

    // Getters
    
    // Setters
    void setPlayer(Entity *p);
    void setAgents(vector<Entity> *a);
};

// Constructors
Rules::Rules() {
    state = SPLASH; // init game state
    
    player = NULL; // set by main
    agents = NULL; // set by main
}

// Methods
void Rules::update() {
    static unsigned int frames = 0;
    static int i = 0;

    switch (state) {
        case SPLASH:
            splash();
	    break;
	case CSEL:
	    characterSelect();
	    break;
	case RACE:
	    race();
	    break;
	case FREE_FLY:
	    freeFly();
	    break;
	case TIME_TRIAL:
	    timeTrial();
	    break;
	case FINISH:
	    finish();
	    break;
	case LEADERBOARD:
	    leaderboard();
	    break;
	default:
	    state = SPLASH;
    }

    frames++;
}

void Rules::splash() {
    // Draw camera above overlooking track
    // Have AI players running track endlessly
    // Have on-screen text w/ logo and 'press any key to start'
    // Once game started, switch state to character select
}

void Rules::characterSelect() {
    // same camera as splash state
    // draw large character plane in front of screen
    // accept left and right arrows to change character
    // accept spacebar to select character
    // Upon accept, switch state to race, free fly, or time trial
}

void Rules::race() {
    // Setup all racers in starting positions
    // Change camera to player camera
    // Run a countdown before allowing the racers to control
    //  (so before giving player control and before starting AI update)
    // Keep track of racer positions
    // When all racers have finish 3 laps, set state to finish
}

void Rules::freeFly() {
    // Setup player in starting position
    // Change camera to player camera
    // No countdown, give player control immediately
    // Accept esc to exit back to splash
    // on esc set state to splash
}

void Rules::timeTrial() {
    // Setup player in starting position
    // Change camera to player camera
    // Countdown before allowing player control
    // Track lap times
    // Upon finishing 3 laps, record race time
    // When 3 laps are finished set state to finish
}

void Rules::finish() {
    // Change camera to splash cam
    // Run AI players running track endlessly
    // Draw the top racer on the screen
    // Draw their names as text, draw their best lap and overall race time as text
    // Prompt for any key to move forward
    // On key press, change state to leaderboard
}

void Rules::leaderboard() {
    // Clear screen in front of splash cam
    // AI players running track in background
    // 2 sections: best lap times and best avg race finish
    // List top 3 racers in each as well as the value
    // Prompt for any button to move forward
    // On key press, change state to splash
}

// Getters

// Setters
void Rules::setPlayer(Entity *p) {
    player = p;
}

void Rules::setAgents(vector<Entity> *a) {
    agents = a;
}

#endif
