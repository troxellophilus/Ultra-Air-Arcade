/*
 * Defines a character in the game world.
 */

#ifndef CHARACTER_H
#define CHARACTER_H

#include <glm/glm.hpp>

class Character {
private:
	Object *obj;   // obj vertices
	glm::vec3 pos; // translation transform for current position
	glm::vec3 vel; // vx, vy, vz of the character in units/s
	glm::vec3 ori; // orientation of the character
	glm::vec4 mat; // material of the character (ka, kd, ks, sh)
	float timeLast;

public:
	Character(Object *, glm::vec3, glm::vec3, glm::vec3, glm::vec4);
	Object * getObject();
	glm::vec3 getPosition();
	glm::vec3 getVelocity();
	glm::vec3 getOrientation();
	glm::vec4 getMaterial();

	void setObject(Object *);
	void setPosition(glm::vec3);
	void setVelocity(glm::vec3);
	void setOrientation(glm::vec3);
	void setMaterial(glm::vec4 material);

	void update();
};

Character::Character(Object *object, glm::vec3 position, glm::vec3 velocity,
			glm::vec3 orientation, glm::vec4 material) {
	setObject(object);
	setPosition(position);
	setVelocity(velocity);
	setOrientation(orientation);
	setMaterial(material);
}

void Character::update() {
	float timeNow, elapsed;

	timeNow = glfwGetTime();
	elapsed = timeNow - timeLast;

	pos.x += (vel.x ? (vel.x / elapsed) : 0);
	pos.y += (vel.y ? (vel.y / elapsed) : 0);
	pos.z += (vel.z ? (vel.z / elapsed) : 0);

	timeLast = timeNow;
}

Object * Character::getObject() {
	return obj;
}

glm::vec3 Character::getPosition() {
	return pos;
}

glm::vec3 Character::getVelocity() {
	return vel;
}

glm::vec3 Character::getOrientation() {
	return ori;
}

glm::vec4 Character::getMaterial() {
	return mat;
}

void Character::setObject(Object *object) {
	obj = object;
}

void Character::setPosition(glm::vec3 position) {
	pos = glm::vec3(position);
}

void Character::setVelocity(glm::vec3 velocity) {
	vel = glm::vec3(velocity);
}

void Character::setOrientation(glm::vec3 orientation) {
	ori = glm::vec3(orientation);
}

void Character::setMaterial(glm::vec4 material) {
	mat = material;
}

#endif
