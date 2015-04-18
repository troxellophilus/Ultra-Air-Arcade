#ifndef COLLISION_HPP
#define COLLISION_HPP

#include <glm/glm.hpp>
#include "types.h"
#include "Character.hpp"

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
    int sample(glm::vec3, Character*);
    int sampleCharacters(Character*, Character*);
    int getScore();
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

int Collision::sample(vec3 player, Character* obj) {
    if (detectCollision(player, obj->getPosition())) {
        if (!cflag && !obj->getCFlag()) {
            cflag = true;
            obj->setCFlag();
            numCollisions++;
        }
        return status::COLLISION;
    }
    else {
        cflag = false;
        return status::OK;
    }
    
}

int Collision::sampleCharacters(Character* obj1, Character* obj2) {
    if (detectCollision(obj1->getPosition(), obj2->getPosition())) {
        if (!bflag && !obj2->getBFlag()) {
            bflag = true;
            obj2->setBFlag(true);
            obj1->setBFlag(true);
            //numCollisions++;
        }
        return status::COLLISION;
    }
    else {
        obj2->setUFlag();
        obj2->setUFlag();
        bflag = false;
        return status::OK;
    }
}

int Collision::getScore() {
    return numCollisions;
}

#endif
