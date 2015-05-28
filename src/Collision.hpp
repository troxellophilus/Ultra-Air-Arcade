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

using namespace std;

class Collision {
private:
    bool cflag;		// True if collision detected.
    bool ptFlag; // True if player is being reset.
    Terrain *terrain; // Pointer to the terrain.
    Entity *player;
    std::vector<Entity> *opponents;
    int resetStep;
    glm::vec3 convertedNor;
    
public:
    Collision();
    Collision(Terrain *terrainPointer, Entity *playerPointer, vector<Entity> *opponentsPointer);
    virtual ~Collision();
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
}

Collision::Collision(Terrain *terrainPointer, Entity *playerPointer, vector<Entity> *opponentsPointer) {
    cflag = false;
    ptFlag = false;
    terrain = terrainPointer;
    player = playerPointer;
    opponents = opponentsPointer;
    resetStep = 0;
}

Collision::~Collision() { }

void Collision::update() {
    if (detectTerrainCollision(player)) {
        ptFlag = true;
        //player->setMaterial(Materials::red);
        glm::vec3 playerPos = player->getPosition();
        Eigen::Vector3f convertedPos = Eigen::Vector3f(playerPos.x, playerPos.y, playerPos.z);
        Eigen::Vector3f normalVec = terrain->getNormal(convertedPos);
        convertedNor = glm::vec3(normalVec(0), normalVec(1), normalVec(2));
        // camera->setMode(Camera::CameraMode::FREE);
        // for (int i = 0; i < 100; i++)
        //     camera->move(Camera::CameraDirection::BACK);
    }

    if (ptFlag) {
        glm::vec3 playerPos = player->getPosition();
        player->setPosition(playerPos + (convertedNor * 0.0167f));
        player->setThrust(0.f);
        player->setVelocity(glm::vec3(0.f, 0.f, 0.f));

        // if (resetStep % 20 == 0) {
        //     player->setMaterial(Materials::red);
        // }

        // else if (resetStep % 10 == 0) {
        //     player->setMaterial(Materials::stone);
        // }

        if (resetStep++ > 180) {
            ptFlag = false;
            resetStep = 0;
            player->setMaterial(Materials::emerald);
            // camera->setMode(Camera::CameraMode::TPC);
        }
    }
}

bool Collision::detectEntityCollision(Entity *player, Entity *object) {

    glm::vec3 playerPosition = player->getPosition();
    glm::vec3 objectPosition = object->getPosition();
    float playerRadius = player->getRadius();
    float objectRadius = object->getRadius();

    // Single operation
    // Spatial data structure for powerups and terrain objects
    // if (fabs(objectPosition.x - playerPosition.x) > (objectRadius + playerRadius))
    //     xOverlap = false;
    // if (fabs(objectPosition.y - playerPosition.y) > (objectRadius + playerRadius))
    //     yOverlap = false;
    // if (fabs(objectPosition.z - playerPosition.z) > (objectRadius + playerRadius))
    //     zOverlap = false;

    return glm::distance(objectPosition, playerPosition) > (objectRadius + playerRadius);

    //cout << xOverlap << " " << yOverlap << " " << zOverlap << endl;
    
    //anyOverlap = xOverlap && yOverlap && zOverlap;
    
    //return anyOverlap;
}

bool Collision::detectTerrainCollision(Entity *object) {
    //return false;

    Eigen::Vector3f convertedVector = Eigen::Vector3f(object->getPosition().x, object->getPosition().y, object->getPosition().z);
    return terrain->detectCollision(convertedVector, object->getRadius());
}

#endif
