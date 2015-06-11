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
#include "RacerAI.hpp"
#include "PlaneSound.hpp"

class Rules {
public:
	enum GameState { SPLASH, CSEL, SETUP, RACE, FREE_FLY, TIME_TRIAL, FINISH, LEADERBOARD };
	
	// Constructors
	Rules();
	
	// Methods
	void update(Camera *);
	
	// Getters
	GameState getState();
	
	// Setters
	void setPlayer(Entity *p);
	void setAgents(vector<Entity> *a);
	void setState(GameState s);
	
private:
	// Mode
	GameState state;
	
	// Players
	Entity *player; // Pointer to the player entity
	RacerAI *playerAI;
	vector<Entity> *agents; // Pointer to the vector of agents in main
	vector<RacerAI *> agentsAI;
	
	PlaneSound countdown = PlaneSound("../Assets/sound/beep.wav");
	
	// state methods
	void splash(Camera *);
	void characterSelect(Camera *);
	void setup(Camera *);
	void race(Camera *);
	void freeFly();
	void timeTrial();
	void finish(Camera *);
	void leaderboard();
};

// Constructors
Rules::Rules() {
	state = SPLASH; // init game state
}

// Methods
void Rules::update(Camera *cam) {
	static unsigned int frames = 0;
	static int i = 0;
	
	//if (frames % 50 == 0)
	//printf("Game State: %d\n", state);
	
	switch (state) {
		case SPLASH:
			splash(cam);
			break;
		case CSEL:
			characterSelect(cam);
			break;
		case SETUP:
			setup(cam);
			break;
		case RACE:
			race(cam);
			break;
		case FREE_FLY:
			freeFly();
			break;
		case TIME_TRIAL:
			timeTrial();
			break;
		case FINISH:
			finish(cam);
			break;
		case LEADERBOARD:
			leaderboard();
			break;
		default:
			state = SPLASH;
	}
	
	frames++;
}

void Rules::splash(Camera *cam) {
	// Draw camera above overlooking track
	cam->setMode(Camera::SPLASH_CAM);
	
	// Have AI players running track endlessly
	for (RacerAI *opp : agentsAI) {
		opp->setState(RacerAI::SPLASH);
	}
	
	// Have on-screen text w/ logo and 'press any key to start'
	playerAI->setState(RacerAI::SPLASH);
	
	// Once game started, switch state to character select
	// This occurs as a result of input handler
}

void Rules::characterSelect(Camera *cam) {
	// same camera as splash state
	cam->setMode(Camera::SPLASH_CAM);
	
	// draw large character plane in front of screen
	player->setPosition(cam->getPosition() + glm::vec3(0, 0, -0.3));
	
	// accept left and right arrows to change character
	// handled by input handler
	
	// accept spacebar to select character
	// handled by input handler
	
	// Upon accept, switch state to race, free fly, or time trial
	// handled by input handler
}

void Rules::setup(Camera *cam) {
	static int start_count = 0;
	
	// Setup all racers in starting positions
	if (start_count == 0) {
		playerAI->setState(RacerAI::SETUP);
		
		for (RacerAI *opp : agentsAI) {
			opp->setState(RacerAI::SETUP);
		}
		
		cam->setMode(Camera::TPC);
		
		printf("RACE BEGIN!\n5\n");
		countdown.setPitch(1.f);
		countdown.play();
	}
	
	start_count++;
	
	if (start_count == 30)
		printf("4\n");
	
	if (start_count == 60) {
		printf("3\n");
		countdown.play();
	}
	
	if (start_count == 90)
		printf("2\n");
	
	if (start_count == 120) {
		printf("1\n");
		countdown.play();
	}
	
	if (start_count == 180) {
		printf("GO!\n");
		countdown.setPitch(2.f);
		countdown.play();
		
		playerAI->setState(RacerAI::RACE);
		for (RacerAI *opp : agentsAI) {
			opp->setState(RacerAI::RACE);
		}
		
		state = RACE;
		start_count = 0;
	}
}

void Rules::race(Camera *cam) {
	static int frames = 0;
	
	if (frames == 0) {
		playerAI->setState(RacerAI::RACE);
		for (RacerAI *opp : agentsAI) {
			opp->setState(RacerAI::RACE);
		}
	}
	
	// Keep track of racer positions
	
	// Trigger avoidance states
	for (Entity opp1 : *agents) {
		if (((RacerAI *)opp1.getAI())->getState() == RacerAI::BOUNCE)
			continue;

		for (Entity opp2 : *agents) {
			float d = glm::distance(opp1.getPosition(), opp2.getPosition());
			if (d < 1.f && d > 0.001f &&
				((RacerAI *)opp2.getAI())->getState() != RacerAI::BOUNCE) {
				((RacerAI *)opp1.getAI())->setAvoidTarget(&opp2);
				((RacerAI *)opp1.getAI())->setState(RacerAI::AVOID);
				((RacerAI *)opp2.getAI())->setAvoidTarget(&opp1);
				((RacerAI *)opp2.getAI())->setState(RacerAI::AVOID);
			}
			//else {
			//	((RacerAI *)opp1.getAI())->setState(RacerAI::RACE);
			//	((RacerAI *)opp2.getAI())->setState(RacerAI::RACE);
			//}
		}
	}
	
	// Update player placement
	int player_next_idx = playerAI->getNextIdx();
	float player_dist = glm::distance(global_track[player_next_idx], player->getPosition());
	
	int num_ahead = 0;
	for (Entity opp : *agents) {
		if (((RacerAI *)opp.getAI())->getLap() > playerAI->getLap())
			num_ahead++;
		else if (((RacerAI *)opp.getAI())->getLap() == playerAI->getLap()) {
			if (((RacerAI *)opp.getAI())->getNextIdx() > player_next_idx)
				num_ahead++;
			else if (((RacerAI *)opp.getAI())->getNextIdx() == player_next_idx) {
				float d = glm::distance(opp.getPosition(), global_track[player_next_idx]);
				if (d < player_dist)
					num_ahead++;
			}
		}
	}
	playerAI->setPlace(num_ahead + 1);
	
	// When all racers have finish 3 laps, set state to finish
	if (playerAI->getLap() == 3) {
		state = FINISH;
		frames = 0;
		printf("The race is complete!\nYou finished in %d place!\n", playerAI->getPlace());
	}
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

void Rules::finish(Camera *cam) {
	// Change camera to splash cam
	cam->setMode(Camera::SPLASH_CAM);
	
	// Draw the top racer on the screen
	
	// Draw their names as text, draw their best lap and overall race time as text
	
	// Prompt for any key to move forward
	
	// On key press, change state to leaderboard
	// Handled by input handler
}

void Rules::leaderboard() {
	// Clear screen in front of splash cam
	
	// 2 sections: best lap times and best avg race finish
	
	// List top 3 racers in each as well as the value
	
	// Prompt for any button to move forward
	
	// On key press, change state to splash
	// Handled by input handler
}

// Getters
Rules::GameState Rules::getState() {
	return state;
}

// Setters
void Rules::setState(GameState s) {
	state = s;
}

void Rules::setPlayer(Entity *p) {
	player = p;
	playerAI = (RacerAI *)(p->getAI());
}

void Rules::setAgents(vector<Entity> *a) {
	agents = a;
	for (Entity e : *a)
		agentsAI.push_back((RacerAI *)(e.getAI()));
}

#endif
