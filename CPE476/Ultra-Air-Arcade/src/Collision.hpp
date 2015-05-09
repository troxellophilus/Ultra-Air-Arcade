#ifndef COLLISION_HPP
#define COLLISION_HPP

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cassert>
#include <cmath>
#include <stdio.h>
#include "GLSL.h"

#include "glm/glm.hpp"
#include "types.h"
#include "Entity.hpp"

#define OBJ_RADIUS 0.5

class Collision {
private:
    bool cflag;		// True if collision detected.
    bool bflag;
    bool uflag;
    bool dir;		// True for in, false for out
    unsigned int numCollisions;
    bool detectCollision(glm::vec3, glm::vec3);
    
public:
    enum status {
        OK = 0,
        COLLISION = 1
    };
    
    Collision();
    virtual ~Collision();
    int sample(glm::vec3, Entity*);
    int sampleEntitys(Entity*, Entity*);
    unsigned int getScore();
};

using namespace std;
using namespace glm;

Collision::Collision() {
    cflag = false;
    bflag = false;
    numCollisions = 0;
}

Collision::~Collision() { }

bool Collision::detectCollision(vec3 player, vec3 object) {
    //return distance(player, object) <= 0.4;
    
    bool xOverlap = true;
    bool yOverlap = true;
    bool zOverlap = true;
    bool anyOverlap = false;
    
    if (fabs(object.x - player.x) > (OBJ_RADIUS + OBJ_RADIUS))
        xOverlap = false;
    if (fabs(object.y - player.y) > (OBJ_RADIUS + OBJ_RADIUS))
        yOverlap = false;
    if (fabs(object.z - player.z) > (OBJ_RADIUS + OBJ_RADIUS))
        zOverlap = false;
    
    //cout << xOverlap << " " << yOverlap << " " << zOverlap << endl;
    
    anyOverlap = xOverlap && yOverlap && zOverlap;
    
    return anyOverlap;
}

int Collision::sample(vec3 player, Entity* obj) {
    if (detectCollision(player, obj->getPosition())) {
	    unsigned int flags = obj->getFlag();
        if (!cflag && !(flags & C_FLAG)) {
            cflag = true;
            obj->setFlag(C_FLAG);
            numCollisions++;
        }
        return COLLISION;
    }
    else {
        cflag = false;
        return OK;
    }
    
}

int Collision::sampleEntitys(Entity* obj1, Entity* obj2) {
    if (detectCollision(obj1->getPosition(), obj2->getPosition())) {
	    unsigned int flags = obj2->getFlag();
        if (!bflag && !(flags & B_FLAG)) {
            bflag = true;
            obj2->setFlag(B_FLAG);
            obj1->setFlag(B_FLAG);
            //numCollisions++;
        }
        return COLLISION;
    }
    else {
        obj2->setFlag(U_FLAG);
        obj1->setFlag(U_FLAG);
        bflag = false;
        return OK;
    }
}

unsigned int Collision::getScore() {
    return numCollisions;
}

#endif
