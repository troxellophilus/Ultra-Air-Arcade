/*
 * Defines an entity in the game world.
 */

#ifndef ENTITY_H
#define ENTITY_H

#include <cstdlib>
#include <glm/glm.hpp>
#include "Materials.hpp"
#include "types.h"

#define NUM_OBJ 5    // The maximum number of objects a character

#define C_FLAG 0x01
#define B_FLAG 0x02
#define U_FLAG 0x04
#define R_FLAG 0x08

using namespace std;

class Entity {
private:
    Object *obj[NUM_OBJ];   // obj vertices
    glm::vec3 pos[NUM_OBJ]; // translation transform for current position
    glm::vec3 vel; // vx, vy, vz of the character in units/s
    glm::vec3 ori;
    Material mat[NUM_OBJ];
    uint8_t flgs = 0x0;
    
public:
    // Constructor
    Entity(vector<Object *>, vector<glm::vec3>);

    // Getters
    Object * getObject(int which);
    glm::vec3 getPosition(int which);
    glm::vec3 getVelocity();
    glm::vec3 getOrientation();
    Material getMaterial(int which);
    uint8_t getFlags();

    // Setters
    void setPosition(int which, glm::vec3);
    void setObject(int which, Object *);
    void setVelocity(glm::vec3);
    void setMaterial(int which, Material material);
    void setOrientation(glm::vec3);
    void setFlag(uint8_t flg);

    // Methods
    void update(double);
};

Entity::Entity(vector<Object *> objects, vector<glm::vec3> positions) {
    for (int i = 0; i < objects.size(); i++) {
	obj[i] = objects[i];
	pos[i] = glm::vec3(positions[i]);
    }
    vel = glm::vec3(0);
    ori = glm::vec3(0, 5, 0);
    flgs = 0x00;
}

void Entity::update(double elapsed) {
    if (flgs & C_FLAG) {
        setMaterial(1, Materials::emerald);
        setVelocity(glm::vec3(0));
    }
    else if (flgs & B_FLAG && !(flgs & R_FLAG)) {
        setVelocity(glm::vec3(0) - vel);
	flgs |= R_FLAG;
    }
    else {
        if (flgs & R_FLAG && flgs & U_FLAG) {
		flgs &= ~(R_FLAG | U_FLAG | B_FLAG);
        }
        pos[0].x += (vel.x ? (vel.x / elapsed) : 0);
        pos[0].y += (vel.y ? (vel.y / elapsed) : 0);
        pos[0].z += (vel.z ? (vel.z / elapsed) : 0);
	pos[1].x = pos[0].x;
	pos[1].z = pos[0].z;
        
	ori.y += 0.25;
    }
}

Object * Entity::getObject(int which) {
    return obj[which];
}

glm::vec3 Entity::getPosition(int which) {
    return pos[which];
}

glm::vec3 Entity::getVelocity() {
    return vel;
}

glm::vec3 Entity::getOrientation() {
    return ori;
}

Material Entity::getMaterial(int which) {
    return mat[which];
}

void Entity::setObject(int which, Object *object) {
    obj[which] = object;
}

void Entity::setPosition(int which, glm::vec3 position) {
    pos[which] = glm::vec3(position);
}

void Entity::setVelocity(glm::vec3 velocity) {
    vel = glm::vec3(velocity);
}

void Entity::setOrientation(glm::vec3 orientation) {
    ori = glm::vec3(orientation);
}

void Entity::setMaterial(int which, Material material) {
    mat[which] = material;
}

void Entity::setFlag(uint8_t flag) {
	flgs |= flag;
}

uint8_t Entity::getFlags() {
	return flgs;
}

#endif
