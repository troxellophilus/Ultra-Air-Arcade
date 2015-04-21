#ifndef COLLISION_HPP
#define COLLISION_HPP

#include <glm/glm.hpp>
#include "types.h"
#include "Entity.hpp"

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
    return distance(player, object) <= 0.4;
}

int Collision::sample(vec3 player, Entity* obj) {
    if (detectCollision(player, obj->getPosition())) {
	    uint8_t flags = obj->getFlags();
        if (!cflag && !(flags & C_FLAG)) {
            cflag = true;
            obj->setFlag(C_FLAG);
            numCollisions++;
        }
        return status::COLLISION;
    }
    else {
        cflag = false;
        return status::OK;
    }
    
}

int Collision::sampleEntitys(Entity* obj1, Entity* obj2) {
    if (detectCollision(obj1->getPosition(), obj2->getPosition())) {
	    uint8_t flags = obj2->getFlags();
        if (!bflag && !(flags & B_FLAG)) {
            bflag = true;
            obj2->setFlag(B_FLAG);
            obj1->setFlag(B_FLAG);
            //numCollisions++;
        }
        return status::COLLISION;
    }
    else {
        obj2->setFlag(U_FLAG);
        obj1->setFlag(U_FLAG);
        bflag = false;
        return status::OK;
    }
}

unsigned int Collision::getScore() {
    return numCollisions;
}

#endif
