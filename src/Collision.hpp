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
    Entity *player;
    vector<Entity> *opponents;
    PlaneSound *planeSound;
    int resetStep;
    int enemyStep;
    glm::vec3 convertedNor;
    PlaneSound *collisionSound;
    
public:
    Collision();
    Collision(Terrain *terrainPointer, Entity *playerPointer, vector<Entity> *opponentsPointer);
    virtual ~Collision();
    void setOpponents(vector<Entity> *opp);
    void setPlayerSound(PlaneSound *sound);
    void setCollisionSound(PlaneSound *sound);
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
}

Collision::Collision(Terrain *terrainPointer, Entity *playerPointer, vector<Entity> *opponentsPointer) {
    cflag = false;
    ptFlag = false;
    terrain = terrainPointer;
    player = playerPointer;
    opponents = opponentsPointer;
    resetStep = 0;
    enemyStep = 0;
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

void Collision::update() {
    // A soft ceiling for the player
    if (player->getPosition().y > 50.f) {
        player->setPosition(glm::vec3(player->getPosition().x, 50.f, player->getPosition().z));
    }

    if (detectTerrainCollision(player)) {
        player->collisionFlag = true;
        player->setMaterial(Materials::red);
        glm::vec3 playerPos = player->getPosition();
        Eigen::Vector3f convertedPos = Eigen::Vector3f(playerPos.x, playerPos.y, playerPos.z);
        Eigen::Vector3f normalVec = terrain->getNormal(convertedPos);
        convertedNor = glm::vec3(normalVec(0), normalVec(1), normalVec(2));
        planeSound->changePitch(0);
        collisionSound->play();
        // camera->setMode(Camera::CameraMode::FREE);
        // for (int i = 0; i < 100; i++)
        //     camera->move(Camera::CameraDirection::BACK);
    }

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

        if (resetStep++ > 120) {
            player->collisionFlag = false;
            resetStep = 0;
            player->setMaterial(Materials::emerald);
            // camera->setMode(Camera::CameraMode::TPC);
        }
    }

    for (Entity opp : *opponents) {
        if (detectEntityCollision(player, &opp)) {
            if (opp.getPosition().x == opp.getPosition().x) {
                if (opp.getPosition().y > player->getPosition().y) {
                    player->setPosition(glm::vec3(player->getPosition().x, opp.getPosition().y - 0.001f, player->getPosition().z));
                    //player->pitch(20.f);
                }
                else {
                    player->setPosition(glm::vec3(player->getPosition().x, opp.getPosition().y + 0.001f, player->getPosition().z));
                    //player->pitch(-20.f);
                }
            }
        }

        for (Entity opp1 : *opponents) {
            if (opp1.getPosition() != opp.getPosition()) {
                if (detectEntityCollision(&opp, &opp1)) {
                    if (opp.getPosition().y > opp1.getPosition().y) {
                        opp.pitch(20.f);
                        opp1.pitch(-20.f);
                    }
                    else {
                        opp.pitch(-20.f);
                        opp1.pitch(20.f);
                    }
                }
            }
        }

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
    float playerRadius = player->getRadius();
    float objectRadius = object->getRadius();

    // Single operation
    // Spatial data structure for powerups and terrain objects
    if (fabs(objectPosition.x - playerPosition.x) > (objectRadius + playerRadius))
        xOverlap = false;
    if (fabs(objectPosition.y - playerPosition.y) > (objectRadius + playerRadius))
        yOverlap = false;
    if (fabs(objectPosition.z - playerPosition.z) > (objectRadius + playerRadius))
        zOverlap = false;

    //return glm::distance(objectPosition, playerPosition) > (objectRadius + playerRadius);

    //cout << xOverlap << " " << yOverlap << " " << zOverlap << endl;
    
    anyOverlap = xOverlap && yOverlap && zOverlap;
    
    return anyOverlap;
}

bool Collision::detectTerrainCollision(Entity *object) {
    //return false;

    Eigen::Vector3f convertedVector = Eigen::Vector3f(object->getPosition().x, object->getPosition().y, object->getPosition().z);
    return terrain->detectCollision(convertedVector, object->getRadius());
}

#endif
