/*
 * Class to define and implement a Racer AI component
 * Drew Troxell
 */

#ifndef RACER_AI_H
#define RACER_AI_H

#include <random>

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Entity.hpp"

#define TRACK_LOCS 30

// Track Target Positions
glm::vec3 track[TRACK_LOCS] = {
	glm::vec3(198.505310, 21.487150, 187.099915),
	glm::vec3(207.387207, 23.095654, 169.286743),
	glm::vec3(218.325424, 19.567459, 161.386002),
	glm::vec3(234.446899, 18.463232, 161.541306),
	glm::vec3(254.835068, 19.991716, 165.367523),
	glm::vec3(278.901001, 20.833832, 172.109802),
	glm::vec3(301.086182, 20.086279, 177.264984),
	glm::vec3(326.090973, 17.922951, 185.066071),
	glm::vec3(340.420288, 18.532957, 191.676743),
	glm::vec3(348.766602, 21.188234, 203.070541),
	glm::vec3(348.444061, 20.676977, 220.082138),
	glm::vec3(338.234436, 18.468874, 237.565399),
	glm::vec3(328.058472, 17.885183, 256.506531),
	glm::vec3(319.400726, 14.683078, 276.147034),
	glm::vec3(315.260590, 18.380798, 293.262299),
	glm::vec3(308.960175, 17.948874, 308.336884),
	glm::vec3(286.623596, 21.178391, 316.497955),
	glm::vec3(272.958771, 21.630711, 323.802795),
	glm::vec3(259.045654, 21.691744, 334.997498),
	glm::vec3(246.990143, 22.934418, 339.439331),
	glm::vec3(235.840179, 23.437473, 337.002716),
	glm::vec3(221.498016, 22.827761, 326.159454),
	glm::vec3(204.109558, 25.518103, 318.211884),
	glm::vec3(193.024582, 26.580734, 323.754486),
	glm::vec3(180.519318, 30.484377, 325.014496),
	glm::vec3(173.635376, 33.372467, 310.009033),
	glm::vec3(171.480789, 31.497810, 291.213501),
	glm::vec3(174.789032, 29.148275, 271.474487),
	glm::vec3(170.541916, 23.302832, 238.429688),
	glm::vec3(175.815781, 19.949869, 214.720856),
};

class RacerAI : public AIComponent {
public:
    enum AIState { SPLASH, SETUP, RACE, AVOID, RECOVER, FINISH };
    enum AIType { PLAYER, OPPONENT, PROP };

    // Constructors
    RacerAI();
    
    // Methods
    virtual void update(void *agent);

    // Setters
    void setState(AIState s);
    void setType(AIType t);
    void setAvoidTarget(Entity *);

    // Getters
    int getLap();

private:
    // Race Status Vars
    int lap;
    int place;
    int track_idx;
    int next_idx;
    int state;
    int type;
    glm::vec3 start_loc;
    Entity *avoid_target;

    void splash(Entity *);
    void setup(Entity *);
    void race(int, Entity *);
    void avoid(Entity *);
    void recover(Entity *);
    void finish(Entity *);
};

// Constructors
RacerAI::RacerAI() {
    lap = 0;
    place = 0;
    track_idx = 0;
    next_idx = 1;
    type = OPPONENT;
    state = SETUP;

    avoid_target = NULL;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> noise(0,1.0);

    start_loc = track[TRACK_LOCS - 1] + glm::vec3(float(noise(gen)), float(noise(gen)), float(noise(gen)));
}

// Methods
void RacerAI::update(void *e) {
    static unsigned int frames = 0;

    Entity *agent = (Entity *)e;

    //if (frames % 30) {
	//printf("AI Type: %d\n", type);
        //printf("AI State: %d\n", state);
    //}

    switch (state) {
	case SPLASH:
	    splash(agent);
            break;
        case SETUP:
	    setup(agent);
	    break;
	case RACE:
	    race(frames, agent);
	    break;
	case AVOID:
	    avoid(agent);
	    break;
	case RECOVER:
	    recover(agent);
	    break;
	case FINISH:
	    finish(agent);
	    break;
	default:
	    state = SETUP;
	    break;
    }

    frames++;
}

void RacerAI::splash(Entity *agent) {

}

void RacerAI::setup(Entity *agent) {
    lap = 0;
    place = 0;
    track_idx = 0;
    next_idx = 1;
    agent->setPosition(start_loc);

    if (agent->getThrust() > -0.5f)
	agent->throttleUp();
}

void RacerAI::race(int frames, Entity *agent) {
    static glm::vec3 target = track[track_idx];

        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> noise(0,2.0);
        target = track[track_idx] + glm::vec3(float(noise(gen)), float(noise(gen)), float(noise(gen)));

    // Update track targets
    if (glm::distance(agent->getPosition(), target) < 5.f) {
        track_idx = next_idx;
        next_idx++;

        if (next_idx > TRACK_LOCS - 1) {
            next_idx = 0;
	    lap++;
	}
    }

    if (type == OPPONENT) {
        agent->throttleUp();

        glm::vec3 todir = glm::normalize(target - agent->getPosition());
        glm::quat q = glm::rotation(glm::vec3(0, 0, -1), todir);
        agent->setTargetRotationQ(q);
    }
}

void RacerAI::avoid(Entity *agent) {
    agent->throttleDown();

    glm::vec3 vec_away_opp = glm::normalize(agent->getPosition() - avoid_target->getPosition());

    agent->setTargetRotationQ(glm::shortMix(agent->getRotationQ(), glm::rotation(glm::vec3(0, 0, -1), vec_away_opp), 0.3f));

    if (glm::distance(agent->getPosition(), avoid_target->getPosition()) > 3.0f)
	state = RACE;
}

void RacerAI::recover(Entity *agent) {
    // Set agent to last checkpoint
    agent->setPosition(track[track_idx - 1]);
}

void RacerAI::finish(Entity *agent) {
    agent->throttleDown();
}

// Getters
int RacerAI::getLap() {
    return lap;
}

// Setters
void RacerAI::setState(AIState s) {
    state = s;
}

void RacerAI::setType(AIType t) {
    type = t;
}

void RacerAI::setAvoidTarget(Entity *e) {
    avoid_target = e;
}

#endif
