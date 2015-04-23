/*
 * Defines an entity in the game world.
 */

#ifndef ENTITY_H
#define ENTITY_H

#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "Materials.hpp"
#include "types.h"

#define C_FLAG 0x01
#define B_FLAG 0x02
#define U_FLAG 0x04
#define R_FLAG 0x08

#define MAX_SPEED 10
#define ACC_FAC 0.5
#define ROLL_FAC 0.5

using namespace std;

class Entity {
private:
    Object *obj;   // obj vertices
    glm::vec3 pos; // translation transform for current position
    glm::vec3 vet; // Target velocity of the character
    glm::vec3 vel; // vx, vy, vz of the character in units/s
    glm::vec3 pyt; // Target Pitch Yaw (orientation) of the character
    glm::vec3 ori; // Orientation dx, dy, dz
    glm::vec3 sca; // scale
    float spd;
    float spt;
    Material mat;
    uint8_t flgs = 0x0;
    
public:
    // Constructor
    Entity(Object *, glm::vec3, glm::vec3, Material);

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
    void setPitchYawTarget(glm::vec3);
    void setFlag(uint8_t flg);

    // Methods
    void packVertices(vector<float> *, vector<float> *, vector<unsigned int> *);
    void update(double);
    void accelerate();
    void decelerate();
    void rollRight();
    void rollLeft();
};

Entity::Entity(Object * object, glm::vec3 position, glm::vec3 scale, Material material) {
    obj = object;
    pos = glm::vec3(position);
    sca = glm::vec3(scale);
    mat = material;
    vel = glm::vec3(0);
    ori = glm::vec3(0, 1, 0);
    spd = 0;
    spt = 0;
    pyt = glm::vec3(0, 1, 0);
    flgs = 0x00;
}

void Entity::update(double elapsed) {
	static glm::vec3 lastPitchYawErr = pyt - ori;
	static float lastSpdErr = spt - spd;
	glm::vec3 pitchYawErr = pyt - ori;
	float spdErr = spt - spd;

	ori += pitchYawErr + ((lastPitchYawErr - pitchYawErr) / float(elapsed)); // PD control
	spd += spdErr + ((lastSpdErr - spdErr) / float(elapsed)); // PD control

	pos += glm::normalize(ori) * spd / float(elapsed);

	lastPitchYawErr = pitchYawErr;
	lastSpdErr = spdErr;
}

void Entity::accelerate() {
	spt -= (spt > -MAX_SPEED ? ACC_FAC : 0);
}

void Entity::decelerate() {
	spt += (spt < 0 ? ACC_FAC : 0);
}

void Entity::rollRight() {
	glm::mat4 R = glm::rotate(float(ROLL_FAC), ori);
	pyt += glm::vec3(R[2][0], R[2][1], R[2][2]);
}

void Entity::rollLeft() {
	glm::mat4 R = glm::rotate(-float(ROLL_FAC), ori);
	pyt += glm::vec3(R[2][0], R[2][1], R[2][2]);
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

void Entity::setPitchYawTarget(glm::vec3 rz) {
	pyt = glm::vec3(rz);
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
