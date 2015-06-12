#ifndef COLLISION_HPP
#define COLLISION_HPP

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cassert>
#include <cmath>
#include <stdio.h>

#include <glm/glm.hpp>
#include <eigen3/Eigen/Dense>
#include "types.h"
#include "Entity.hpp"
#include "Terrain.h"
#include "RacerAI.hpp"
#include "PlaneSound.hpp"

using namespace std;

class Collision {
private:
    bool cflag;		// True if collision detected.
    bool ptFlag; // True if player is being reset.
    Terrain *terrain; // Pointer to the terrain.
    Entity *player; // Pointer to the player
    vector<Entity> *opponents; // Pointer to the opponents
    PlaneSound *planeSound; // Pointer to the plane sound
    int resetStep; // Frame counter for resetting the player
    int enemyStep; // Frame counter for resetting an opponent
    glm::vec3 convertedNor; // Converted normal vector for colliding with terrain
    PlaneSound *collisionSound; // Pointer to the collision sound
    PlaneSound *enemyCollisionSound; // Pointer to the enemy collision sound
	glm::vec3 sphereTranslations[5];
	float sphereRadius[5];
    
public:
    Collision();
    Collision(Terrain *terrainPointer, Entity *playerPointer, vector<Entity> *opponentsPointer);
    virtual ~Collision();
    void setOpponents(vector<Entity> *opp);
    void setPlayerSound(PlaneSound *sound);
    void setCollisionSound(PlaneSound *sound);
    void setEnemyCollisionSound(PlaneSound *sound);
    void update();
    bool detectEntityCollision(Entity *player, Entity *object);
    bool detectTerrainCollision(Entity *object);
};

//using namespace std;
//using namespace glm;

Collision::Collision() {
    cflag = false;
    ptFlag = false;
    terrain = NULL;
    player = NULL;
    opponents = NULL;
    resetStep = 0;
    enemyStep = 0;

	sphereTranslations[0] = glm::vec3(0.18, -0.025, 0.05);
	sphereRadius[0] = 0.04;
	sphereTranslations[1] = glm::vec3(-0.18, -0.025, 0.05);
	sphereRadius[1] = 0.04;
	sphereTranslations[2] = glm::vec3(0.0, -0.025, -0.2);
	sphereRadius[2] = 0.04;
	sphereTranslations[3] = glm::vec3(0.18, -0.025, 0.05);
	sphereRadius[3] = 0.04;
	sphereTranslations[4] = glm::vec3(0.0);
	sphereRadius[4] = 0.04;
}

Collision::Collision(Terrain *terrainPointer, Entity *playerPointer, vector<Entity> *opponentsPointer) {
    cflag = false;
    ptFlag = false;
    terrain = terrainPointer;
    player = playerPointer;
    opponents = opponentsPointer;
    resetStep = 0;
    enemyStep = 0;

	sphereTranslations[0] = glm::vec3(0.18, -0.025, 0.05);
	sphereRadius[0] = 0.04;
	sphereTranslations[1] = glm::vec3(-0.18, -0.025, 0.05);
	sphereRadius[1] = 0.04;
	sphereTranslations[2] = glm::vec3(0.0, -0.025, -0.2);
	sphereRadius[2] = 0.04;
	sphereTranslations[3] = glm::vec3(0.18, -0.025, 0.05);
	sphereRadius[3] = 0.04;
	sphereTranslations[4] = glm::vec3(0.0);
	sphereRadius[4] = 0.04;
}

Collision::~Collision() { }

void Collision::setOpponents(vector<Entity> *opp) {
    opponents = opp;
}

void Collision::setPlayerSound(PlaneSound *sound) {
    planeSound = sound;
}

void Collision::setCollisionSound(PlaneSound *sound) {
    collisionSound = sound;
}

void Collision::setEnemyCollisionSound(PlaneSound *sound) {
    enemyCollisionSound = sound;
}

void Collision::update() {
    // A soft ceiling for the player
    if (player->getPosition().y > 50.f) {
        player->setPosition(glm::vec3(player->getPosition().x, 50.f, player->getPosition().z));
    }

    // Collision detection for the player against the terrain
    if (detectTerrainCollision(player)) {
        player->collisionFlag = true;
        player->setMaterial(Materials::red);
        glm::vec3 playerPos = player->getPosition();
        Eigen::Vector3f convertedPos = Eigen::Vector3f(playerPos.x, playerPos.y, playerPos.z);
        Eigen::Vector3f normalVec = terrain->getNormal(convertedPos);
        convertedNor = glm::vec3(normalVec(0), normalVec(1), normalVec(2));
        planeSound->changePitch(0);
        collisionSound->play();
    }

    // Collision response for the player against the terrain
    if (player->collisionFlag) {
        glm::vec3 playerPos = player->getPosition();
        player->setPosition(playerPos + (convertedNor * 0.0167f));
        player->setThrust(0.f);
        player->setVelocity(glm::vec3(0.f, 0.f, 0.f));
        planeSound->changePitch(0);

        if (resetStep % 20 == 0) {
            player->setMaterial(Materials::red);

        }

        else if (resetStep % 10 == 0) {
            player->setMaterial(Materials::stone);
        }

        if (resetStep++ > 30) {
            player->collisionFlag = false;
            resetStep = 0;
            player->setMaterial(player->getBaseMaterial());
	    player->setThrust(-1.f);
	    player->setVelocity(glm::vec3(0.f, 0.f, -10.f));
        }
    }

    // Checking for collisions with opponents
    for (Entity opp : *opponents) {
        // Checking for collisions between player and opponents
        if (detectEntityCollision(player, &opp)) {
            if (opp.getPosition().x == opp.getPosition().x) {
                glm::vec3 vec_away_opp = glm::normalize(player->getPosition() - opp.getPosition());
                player->setTargetRotationQ(glm::shortMix(player->getRotationQ(), glm::rotation(glm::vec3(0, 0, -1), vec_away_opp), 0.2f));
		((RacerAI *)opp.getAI())->setBounceTarget(player);
		((RacerAI *)opp.getAI())->setState(RacerAI::BOUNCE);
                enemyCollisionSound->play();
            }
        }

        // Checking for collisions between two opponents
        for (Entity opp1 : *opponents) {
            if (opp1.getPosition() != opp.getPosition()) {
                if (detectEntityCollision(&opp, &opp1)) {
                        // opp.pitch(20.f);
                        // opp1.pitch(-20.f);
			((RacerAI *)opp.getAI())->setBounceTarget(&opp1);
			((RacerAI *)opp.getAI())->setState(RacerAI::BOUNCE);
			((RacerAI *)opp1.getAI())->setBounceTarget(&opp);
			((RacerAI *)opp1.getAI())->setState(RacerAI::BOUNCE);
                }
            }
        }

        // Detecting collisions between opponent and terrain
        if (detectTerrainCollision(&opp)) {
            //opp.collisionFlag = true;
            opp.setMaterial(Materials::red);
            glm::vec3 oppPos = opp.getPosition();
            Eigen::Vector3f convertedPos = Eigen::Vector3f(oppPos.x, oppPos.y, oppPos.z);
            Eigen::Vector3f normalVec = terrain->getNormal(convertedPos);
            glm::vec3 collisionNormal = glm::vec3(normalVec(0), normalVec(1), normalVec(2));
            opp.setPosition(oppPos + (collisionNormal * 2.f));
            opp.setThrust(0.f);
            opp.setVelocity(glm::vec3(0.f, 0.f, 0.f));
        }
    }
}

bool Collision::detectEntityCollision(Entity *player, Entity *object) {

    bool anyOverlap = false;
    bool xOverlap = true;
    bool yOverlap = true;
    bool zOverlap = true;

    glm::vec3 playerPosition = player->getPosition();
    glm::vec3 objectPosition = object->getPosition();
    float playerRadius = player->getRadius() - 0.06f;
    float objectRadius = object->getRadius() - 0.06f;

    // Single operation
    // Spatial data structure for powerups and terrain objects
    if (fabs(objectPosition.x - playerPosition.x) > (objectRadius + playerRadius))
        xOverlap = false;
    if (fabs(objectPosition.y - playerPosition.y) > (objectRadius + playerRadius))
        yOverlap = false;
    if (fabs(objectPosition.z - playerPosition.z) > (objectRadius + playerRadius))
        zOverlap = false;
    
    anyOverlap = xOverlap && yOverlap && zOverlap;
    
	// For hierarchical bounding spheres
	if (anyOverlap) {
		int i;
		for (i = 0; i < 5; i++) {
			glm::quat rot = player->getRotationQ();
			glm::vec3 directionx = sphereTranslations[i].x * glm::normalize(glm::vec3(1, 0, 0) * glm::inverse(rot));
			glm::vec3 directiony = sphereTranslations[i].y * glm::normalize(glm::vec3(0, 1, 0) * glm::inverse(rot));
			glm::vec3 directionz = sphereTranslations[i].z * glm::normalize(glm::vec3(0, 0, 1) * glm::inverse(rot));
			glm::vec3 refPos1 = player->getPosition() + directionx + directiony + directionz;
			if (glm::distance(refPos1, objectPosition) <= sphereRadius[i] + objectRadius) {
				int j;
				for (j = 0; j < 5; j++) {
					rot = object->getRotationQ();
					directionx = sphereTranslations[j].x * glm::normalize(glm::vec3(1, 0, 0) * glm::inverse(rot));
					directiony = sphereTranslations[j].y * glm::normalize(glm::vec3(0, 1, 0) * glm::inverse(rot));
					directionz = sphereTranslations[j].z * glm::normalize(glm::vec3(0, 0, 1) * glm::inverse(rot));
					glm::vec3 refPos2 = object->getPosition() + directionx + directiony + directionz;
					if (glm::distance(refPos1, refPos2) <= sphereRadius[i] + sphereRadius[j]) return true;
				}
			}
		}
	}

    return anyOverlap;
}

bool Collision::detectTerrainCollision(Entity *object) {

    // Converts the object vector to Eigen and sends that to the collision function in terrain
    Eigen::Vector3f convertedVector = Eigen::Vector3f(object->getPosition().x, object->getPosition().y, object->getPosition().z);
    return terrain->detectCollision(convertedVector, object->getRadius());
}

#endif
