/*
 * Defines an entity in the game world.
 */

#ifndef ENTITY_H
#define ENTITY_H

#include <cstdlib>
#include <glm/glm.hpp>
#include "Materials.hpp"
#include "types.h"

#define C_FLAG 0x01
#define B_FLAG 0x02
#define U_FLAG 0x04
#define R_FLAG 0x08

using namespace std;

class Entity {
private:
    Object *obj;   // obj vertices
    glm::vec3 pos; // translation transform for current position
    glm::vec3 vel; // vx, vy, vz of the character in units/s
    glm::vec3 ori;
    glm::vec3 sca; // scale
    Material mat;
    uint8_t flgs = 0x0;
    
public:
    // Constructor
    Entity(Object *, glm::vec3, glm::vec3, Material);
    void packVertices(vector<float> *, vector<float> *, vector<unsigned int> *);

    // Getters
    Object * getObject();
    glm::vec3 getPosition();
    glm::vec3 getScale();
    glm::vec3 getVelocity();
    glm::vec3 getOrientation();
    Material getMaterial();
    uint8_t getFlags();

    // Setters
    void setPosition(glm::vec3);
    void setScale(glm::vec3);
    void setObject(Object *);
    void setVelocity(glm::vec3);
    void setMaterial(Material material);
    void setOrientation(glm::vec3);
    void setFlag(uint8_t flg);

    // Methods
    void update(double);
};

Entity::Entity(Object * object, glm::vec3 position, glm::vec3 scale, Material material) {
    obj = object;
    pos = glm::vec3(position);
    sca = glm::vec3(scale);
    mat = material;
    vel = glm::vec3(0);
    ori = glm::vec3(0, 5, 0);
    flgs = 0x00;
}

void Entity::update(double elapsed) {
    if (flgs & C_FLAG) {
        setMaterial(Materials::emerald);
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
        pos.x += (vel.x ? (vel.x / elapsed) : 0);
        pos.y += (vel.y ? (vel.y / elapsed) : 0);
        pos.z += (vel.z ? (vel.z / elapsed) : 0);
    }
}

void Entity::packVertices(vector<float> *pbo, vector<float> *nbo, vector<unsigned int> *ibo) {
	int iboIdx = 0;
	for (size_t i=0; i < obj->shapes.size(); i++) {
		pbo->insert(pbo->end(), obj->shapes[i].mesh.positions.begin(), obj->shapes[i].mesh.positions.end());
		nbo->insert(nbo->end(), obj->shapes[i].mesh.normals.begin(), obj->shapes[i].mesh.normals.end());
		for (size_t j=0; j < obj->shapes[i].mesh.indices.size(); j++)
			ibo->push_back(iboIdx + obj->shapes[i].mesh.indices[j]);

		iboIdx += obj->shapes[i].mesh.indices.size();
	}
}

Object * Entity::getObject() {
    return obj;
}

glm::vec3 Entity::getPosition() {
    return pos;
}

glm::vec3 Entity::getScale() {
    return sca;
}

glm::vec3 Entity::getVelocity() {
    return vel;
}

glm::vec3 Entity::getOrientation() {
    return ori;
}

Material Entity::getMaterial() {
    return mat;
}

void Entity::setObject(Object *object) {
    obj = object;
}

void Entity::setPosition(glm::vec3 position) {
    pos = glm::vec3(position);
}

void Entity::setScale(glm::vec3 scale) {
    sca = glm::vec3(scale);
}

void Entity::setVelocity(glm::vec3 velocity) {
    vel = glm::vec3(velocity);
}

void Entity::setOrientation(glm::vec3 orientation) {
    ori = glm::vec3(orientation);
}

void Entity::setMaterial(Material material) {
    mat = material;
}

void Entity::setFlag(uint8_t flag) {
	flgs |= flag;
}

uint8_t Entity::getFlags() {
	return flgs;
}

#endif
