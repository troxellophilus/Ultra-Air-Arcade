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
#include "PlaneSound.hpp"

#define TRACK_LOCS 52

// Track Target Positions
glm::vec3 global_track[TRACK_LOCS] = {
glm::vec3(185.700714, 8.031444, 202.729111),
glm::vec3(197.875748, 11.899343, 187.747467),
glm::vec3(203.233734, 11.739080, 176.238144),
glm::vec3(208.486404, 11.327868, 166.264130),
glm::vec3(220.139496, 10.584544, 159.148911),
glm::vec3(234.944946, 7.518212, 156.845123),
glm::vec3(252.579819, 8.884101, 161.820969),
glm::vec3(269.187469, 10.734179, 168.023590),
glm::vec3(290.921051, 9.244589, 174.749664),
glm::vec3(302.556366, 5.069328, 177.352097),
glm::vec3(313.700531, 5.473619, 178.647293),
glm::vec3(325.850281, 7.084900, 181.554565),
glm::vec3(331.592651, 7.918511, 185.334839),
glm::vec3(338.827698, 9.496173, 189.391266),
glm::vec3(347.107697, 10.140229, 195.790192),
glm::vec3(349.499939, 10.932392, 201.928787),
glm::vec3(351.655151, 11.104140, 209.554901),
glm::vec3(351.582550, 9.795603, 216.287674),
glm::vec3(346.579498, 8.759157, 227.580704),
glm::vec3(340.754028, 9.723553, 236.174179),
glm::vec3(334.784027, 9.800739, 247.007889),
glm::vec3(331.836426, 7.716289, 253.882431),
glm::vec3(326.929321, 8.173429, 258.741852),
glm::vec3(322.155670, 9.636541, 263.517548),
glm::vec3(320.182251, 8.608635, 272.187897),
glm::vec3(317.216492, 8.784968, 285.182892),
glm::vec3(315.298523, 10.438764, 295.485626),
glm::vec3(312.885925, 10.453979, 304.156799),
glm::vec3(307.281342, 7.284096, 313.149414),
glm::vec3(297.104004, 7.030801, 315.389648),
glm::vec3(290.118805, 8.352761, 315.191711),
glm::vec3(281.255981, 9.264643, 316.880524),
glm::vec3(273.451111, 7.642698, 322.880981),
glm::vec3(265.550079, 7.056843, 329.940338),
glm::vec3(260.194763, 8.803595, 334.960480),
glm::vec3(254.210236, 10.071442, 340.005798),
glm::vec3(245.117752, 12.090432, 342.984009),
glm::vec3(234.993240, 14.264776, 338.866119),
glm::vec3(229.883667, 13.946325, 333.044556),
glm::vec3(222.722198, 12.384048, 326.021179),
glm::vec3(216.283661, 13.134801, 319.852386),
glm::vec3(204.992233, 15.913347, 315.637451),
glm::vec3(197.058319, 16.784454, 320.800507),
glm::vec3(190.124176, 16.910112, 324.708832),
glm::vec3(180.407349, 17.215569, 325.275391),
glm::vec3(173.255203, 15.882312, 317.669006),
glm::vec3(170.928802, 15.424024, 305.076385),
glm::vec3(171.704132, 17.328259, 293.882233),
glm::vec3(173.500290, 17.169609, 284.294922),
glm::vec3(174.815628, 12.477604, 271.397400),
glm::vec3(172.338562, 9.600041, 242.724686),
glm::vec3(170.056442, 13.324112, 222.544495),
};

class RacerAI : public AIComponent {
public:
    enum AIState { SPLASH, SETUP, RACE, AVOID, BOUNCE, RECOVER, FINISH };
    enum AIType { PLAYER, OPPONENT, PROP };

    // Constructors
    RacerAI();
    
    // Methods
    virtual void update(void *agent);

    // Setters
    void setState(AIState s);
    void setType(AIType t);
    void setAvoidTarget(Entity *);
    void setBounceTarget(Entity *);
    void setPlace(int p);

    // Getters
    int getLap();
    int getNextIdx();
    int getState();

private:
    int id;
    glm::vec3 track[TRACK_LOCS];

    // Race Status Vars
    int lap;
    int place;
    int track_idx;
    int next_idx;
    int state;
    int type;
    glm::vec3 start_loc;
    Entity *avoid_target;
    Entity *bounce_target;

    PlaneSound bink = PlaneSound("../Assets/sound/Ding.wav");

    void splash(Entity *);
    void setup(Entity *);
    void race(int, Entity *);
    void avoid(Entity *);
    void bounce(Entity *);
    void recover(Entity *);
    void finish(Entity *);
};

// Constructors
RacerAI::RacerAI() {
    static int new_id = 1;
    id = new_id++;
    lap = 0;
    place = 0;
    track_idx = 0;
    next_idx = 1;
    type = OPPONENT;
    state = SETUP;

    avoid_target = NULL;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> noise(0,2.0);
    float x = noise(gen);
    float y = noise(gen);
    float z = noise(gen);

    start_loc = global_track[TRACK_LOCS - 1] + glm::vec3(x, y, z);

    // Fill individual track locations with noise
    for (int i = 0; i < TRACK_LOCS; i++)
	track[i] = global_track[i] + glm::vec3(noise(gen), noise(gen), noise(gen));
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
	case BOUNCE:
	    bounce(agent);
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
	// give player default track positions
	if (type == PLAYER) {
		start_loc = global_track[TRACK_LOCS - 1];
    		for (int i = 0; i < TRACK_LOCS; i++)
			track[i] = global_track[i];
	}
}

void RacerAI::setup(Entity *agent) {
    lap = 0;
    place = 0;
    track_idx = 0;
    next_idx = 1;
    agent->setPosition(start_loc);
    agent->setThrust(0);
    agent->setVelocity(glm::vec3(0, 0, 0));
}

void RacerAI::race(int frames, Entity *agent) {
    static glm::vec3 target = track[track_idx];

    target = track[track_idx];// + glm::vec3(x, y, z);

    // Update track targets
    if (glm::distance(agent->getPosition(), target) < 8.f) {
        track_idx = next_idx;
        next_idx++;
        if (type == PLAYER) {
            bink.setVolume(50.f);
            bink.play();
        }

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

	if (type == PLAYER && frames % 50 == 0) {
		printf("Player place: %d\n", place);
		printf("Player lap: %d\n", lap);
	}
}

void RacerAI::avoid(Entity *agent) {
    agent->throttleDown();

    glm::vec3 vec_away_opp = glm::normalize(agent->getPosition() - avoid_target->getPosition());

    agent->setTargetRotationQ(glm::shortMix(agent->getRotationQ(), glm::rotation(glm::vec3(0, 0, -1), vec_away_opp), 0.3f));

    if (glm::distance(agent->getPosition(), avoid_target->getPosition()) > 3.0f)
	state = RACE;
}

void RacerAI::bounce(Entity *agent) {
	static int count = 0;

	glm::vec3 vec_away_agent = glm::normalize(agent->getPosition() - bounce_target->getPosition());
	agent->setTargetRotationQ(glm::shortMix(agent->getRotationQ(), glm::rotation(glm::vec3(0, 0, -1), vec_away_agent), 0.8f));

	count++;

	if (count > 70) {
		count = 0;
		state = RACE;
	}
}

void RacerAI::recover(Entity *agent) {
    // Set agent to last checkpoint
    agent->setPosition(track[track_idx - 1]);
    agent->setVelocity(glm::vec3(0, 0, -3.f));
    state = RACE;
}

void RacerAI::finish(Entity *agent) {
    agent->throttleDown();
}

// Getters
int RacerAI::getLap() {
    return lap;
}

int RacerAI::getNextIdx() {
    return next_idx;
}

int RacerAI::getState() {
	return state;
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

void RacerAI::setBounceTarget(Entity *e) {
	bounce_target = e;
}

void RacerAI::setPlace(int p) {
	place = p;
}

#endif
