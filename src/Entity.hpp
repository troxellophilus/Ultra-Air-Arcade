/*
 * Defines an entity in the game world.
 */

#ifndef ENTITY_H
#define ENTITY_H

#include <cstdlib>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Materials.hpp"
#include "types.h"

enum EntityFlag { C_FLAG, U_FLAG, B_FLAG };

using namespace std;

class Entity {
private:
    Object     *object;  // obj vertices
    Material   material; // Material of the entity

    glm::vec3  position;      // translation transform for current position
    glm::vec3  direction;     // Direction vector of the entity
    glm::quat  rotation;      // Quaternion rotation of entity
    glm::vec3  scale;         // scale

    glm::vec3  velocity;      // x, y, z velocity of the entity u/s
    glm::vec3  acceleration;  // x, y, z acceleration of the entity u/s^2

    EntityFlag flag;     // Entity flags, use as needed
    
public:
    // Constructor
    Entity();

    // Getters
    Object *   getObject();
    Material   getMaterial();

    glm::vec3  getPosition();
    glm::vec3  getScale();
    glm::mat4  getRotationM();
    glm::quat  getRotationQ();

    glm::vec3  getVelocity();
    glm::vec3  getAcceleration();

    EntityFlag getFlag();

    // Setters
    void setObject(Object *);
    void setMaterial(Material material);

    void setPosition(glm::vec3);
    void setScale(glm::vec3);

    void setVelocity(glm::vec3);

    void setFlag(EntityFlag new_flag);

    // Methods
    void update();

    void pitch(float dy);
    void turn(float dx);
    void rollRight();
    void rollLeft();

    void throttleUp();
    void throttleDown();

    void packVertices(vector<float> *, vector<float> *, vector<unsigned int> *);
};

Entity::Entity() {
    object = NULL;
    material = Material::Material();

    position = glm::vec3(0, 0, 0);
    scale = glm::vec3(1, 1, 1);
    rotation = glm::quat(1, 0, 0, 0);

    velocity = glm::vec3(0, 0, 0);
    acceleration = glm::vec3(0, 0, 0.01);

    flag = C_FLAG;
}

void Entity::update() {
	// TODO: Make movement velocity & acceleration based
	// Update the position by moving velocity in direction
	position += rotation * velocity;
}

void Entity::throttleUp() {
	velocity.z -= 0.1;
}

void Entity::throttleDown() {
	velocity.z += 0.1;
}

void Entity::pitch(float dy) {
	dy = dy > 50.f ? 50.f : dy;
	dy = dy < -50.f ? -50.f : dy;

	// Build dy pitch rotation glm::quat around x axis
	glm::quat rot = glm::angleAxis(dy / 360.f, glm::vec3(1, 0, 0));

	// Apply pitch change to the current rotation.
	rotation *= rot;
}

void Entity::rollRight() {
	// build roll quat
	glm::quat rol = glm::angleAxis(-0.1f, glm::vec3(0, 0, 1));

	// Apply roll change
	rotation *= rol;
}

void Entity::rollLeft() {
	// build roll quat
	glm::quat rol = glm::angleAxis(0.1f, glm::vec3(0, 0, 1));

	// Apply roll change
	rotation *= rol;
}

void Entity::turn(float dx) {
	dx = dx > 50.f ? 50.f : dx;
	dx = dx < -50.f ? -50.f : dx;

	// Build dx yaw rotation glm::quat around y axis
	glm::quat rot = glm::angleAxis(-dx / 360.f, glm::vec3(0, 1, 0));

	// Build dx roll rotation glm::quat around z axis
	glm::quat rol = glm::angleAxis(-dx / 360.f, glm::vec3(0, 0, 1));

	// Apply yaw change to the current rotation.
	rotation *= glm::mix(rot, rol, 0.8f);
}

void Entity::packVertices(vector<float> *pbo, vector<float> *nbo, vector<unsigned int> *ibo) {
	int iboIdx = 0;
	for (size_t i=0; i < object->shapes.size(); i++) {
		pbo->insert(pbo->end(), object->shapes[i].mesh.positions.begin(), object->shapes[i].mesh.positions.end());
		nbo->insert(nbo->end(), object->shapes[i].mesh.normals.begin(), object->shapes[i].mesh.normals.end());
		for (size_t j=0; j < object->shapes[i].mesh.indices.size(); j++)
			ibo->push_back(iboIdx + object->shapes[i].mesh.indices[j]);

		iboIdx += object->shapes[i].mesh.indices.size();
	}
}

Object * Entity::getObject() {
    return object;
}

Material Entity::getMaterial() {
    return material;
}

glm::vec3 Entity::getPosition() {
    return position;
}

glm::vec3 Entity::getScale() {
    return scale;
}

glm::mat4 Entity::getRotationM() {
    return glm::toMat4(rotation);
}

glm::quat Entity::getRotationQ() {
	return rotation;
}

glm::vec3 Entity::getVelocity() {
    return velocity;
}

void Entity::setObject(Object *obj) {
    object = obj;
}

void Entity::setPosition(glm::vec3 pos) {
    pos = glm::vec3(pos);
}

void Entity::setScale(glm::vec3 sc) {
    scale = glm::vec3(sc);
}

void Entity::setVelocity(glm::vec3 vel) {
    velocity = glm::vec3(vel);
}

void Entity::setMaterial(Material mat) {
    material = mat;
}

void Entity::setFlag(EntityFlag f) {
	flag = f;
}

EntityFlag Entity::getFlag() {
	return flag;
}

#endif
