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

class Collision {
private:
    bool cflag;		// True if collision detected.
    
public:
    Collision();
    virtual ~Collision();
    bool detectEntityCollision(Entity player, Entity object);
    bool detectTerrainCollision(Entity object, Terrain *terrain);
};

using namespace std;
//using namespace glm;

Collision::Collision() {
    cflag = false;
}

Collision::~Collision() { }

bool Collision::detectEntityCollision(Entity player, Entity object) {
    
    bool xOverlap = true;
    bool yOverlap = true;
    bool zOverlap = true;
    bool anyOverlap = false;

    glm::vec3 playerPosition = player.getPosition();
    glm::vec3 objectPosition = object.getPosition();
    float playerRadius = player.getRadius();
    float objectRadius = object.getRadius();

    // Single operation
    // Spatial data structure for powerups and terrain objects
    if (fabs(objectPosition.x - playerPosition.x) > (objectRadius + playerRadius))
        xOverlap = false;
    if (fabs(objectPosition.y - playerPosition.y) > (objectRadius + playerRadius))
        yOverlap = false;
    if (fabs(objectPosition.z - playerPosition.z) > (objectRadius + playerRadius))
        zOverlap = false;
    
    //cout << xOverlap << " " << yOverlap << " " << zOverlap << endl;
    
    anyOverlap = xOverlap && yOverlap && zOverlap;
    
    return anyOverlap;
}

bool Collision::detectTerrainCollision(Entity object, Terrain *terrain) {
    //return false;

    Eigen::Vector3f convertedVector = Eigen::Vector3f(object.getPosition().x, object.getPosition().y, object.getPosition().z);
    return terrain->detectCollision(convertedVector, object.getRadius());
}

#endif
