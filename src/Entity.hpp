/*
 * Defines an entity in the game world.
 */

#ifndef ENTITY_H
#define ENTITY_H

#include <cstdlib>
#include <cfloat>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Particle.hpp"
#include "Effect.hpp"
#include "Materials.hpp"
#include "types.h"

#define GAME_SCALE 0.01

enum EntityFlag { C_FLAG, U_FLAG, B_FLAG };

enum EntityType { PLAYER_ENTITY, AI_ENTITY, PROP_ENTITY };

using namespace std;

class AIComponent {
public:
	virtual ~AIComponent() {}
	virtual void update(void *e) = 0;
};

class Entity {
private:
	// Object shape & color properties
	Object    *object;  // obj vertices
	Material  material; // Material of the entity
	Material  base_material;
	Particle flame; 	// Billboarded particle that will make up the jet exhause
	Effect effect;
   glm::vec3 scale;    // scale of the object model
	
	// Components
	AIComponent *ai_;
	
	// Position & orientation properties
	glm::vec3 position;        // translation transform for current position
	glm::vec3 direction;       // Direction vector of the entity
	glm::quat rotation;        // Quaternion rotation of entity
	glm::quat target_rotation; // Quaternion target rotation of entity
	
	float pitch_angle; // relative angle from current of most recent cursor input
	
	// Physical properties
	float     mass;  // mass of the plane
	glm::vec3 force; // Force in Z direction correlated to thrust
	float     drag;  // drag coefficient
	float     carea; // cross sectional area
	
	// Movement properties
	float     thrust;   // thrust of the plane
	float     max_thrust;
	glm::vec3 velocity; // x, y, z velocity of the entity u/s
	
	// Flags if necessary
	EntityFlag flag; // Entity flags, use as needed
	EntityType type; // Flag to designate the type of entity
	
	// Bounding sphere radius
	float radius;
	
	// Game state properties
	int ammunition;
	
public:
	// Constructor
	Entity(AIComponent *ai);
	Entity();
	
	// Getters
	Object * getObject();
	Material getMaterial();
	Material getBaseMaterial();
	
	glm::vec3 getPosition();
	glm::vec3 getScale();
	glm::mat4 getRotationM();
	glm::quat getRotationQ();
	glm::vec3 getDirection();
	float getPitch();
	
	glm::vec3 getVelocity();
	float getThrust();
	
	EntityFlag getFlag();
	
	float getRadius();
	
	AIComponent *getAI();
	
	// Setters
	void setObject(Object *);
	void setMaterial(Material material);
	void setBaseMaterial(Material mat);
	
	void setPosition(glm::vec3);
	void setScale(glm::vec3);
	void setTargetRotationQ(glm::quat);
	void setRotationQ(glm::quat);
	
	void setVelocity(glm::vec3);
	void setThrust(float a);
	void setMaxThrust(float a);
	
	void setFlag(EntityFlag new_flag);
	void setType(EntityType new_type);

	void setParticleProg(GLuint);
   void setEffectProg(GLuint);   

	// Methods
	void update();
	void update(glm::mat4, glm::mat4);
   void updateEffect(glm::mat4, glm::mat4);
   void drawEffect();
	void drawExhaust();
	
	void pitch(float dy);
	void yaw(float dx);
	void turn(float dx);
	void rollRight();
	void rollLeft();
	
	void throttleUp();
	void throttleDown();
	
	void packVertices(vector<float> *, vector<float> *, vector<unsigned int> *,std::vector<float> *);
	
	void calculateBoundingSphereRadius();

	void addAmmo(int);
	int getAmmo();
	void subtractAmmo(int);
	
	bool collisionFlag;
};

Entity::Entity() {
	object = NULL;
	material = Material();
	base_material = Material();
	
	position = glm::vec3(0, 0, 0);
	rotation = glm::quat(1, 0, 0, 0);
	target_rotation = glm::quat(1, 0, 0, 0);
	
	
	scale = glm::vec3(1, 1, 1);
	mass = 3000.0f;
	force = glm::vec3(0, 0, 0);
	drag = 1.5f; // sphere for now
	carea = 25.f;
	ammunition = 0;
	
	thrust = 0.f;
	max_thrust = -1.f;
	velocity = glm::vec3(0, 0, 0);
	
	flag = C_FLAG;
	type = AI_ENTITY;
	
	radius = 0.f;
	
	ai_ = NULL;
	
	collisionFlag = false;
}

Entity::Entity(AIComponent *ai) {
	object = NULL;
	// Zach - removed Material:: from the call to the Material constructor -
	// Would not allow my version to compile
	material = Material();
	base_material = Material();
	
	position = glm::vec3(0, 0, 0);
	rotation = glm::quat(1, 0, 0, 0);
	target_rotation = glm::quat(1, 0, 0, 0);
	
	scale = glm::vec3(1, 1, 1);
	mass = 5000.0f;
	force = glm::vec3(0, 0, 0);
	drag = 1.5f; // sphere for now
	carea = 25.f;
	ammunition = 0;
	
	thrust = 0.f;
	max_thrust = -1.f;
	velocity = glm::vec3(0, 0, 0);
	
	flag = C_FLAG;
	type = AI_ENTITY;
	
	radius = 0.f;
	
	ai_ = ai;
	
	collisionFlag = false;
}

void Entity::setParticleProg(GLuint program) {
	flame.setShaderProg(program);
}

void Entity::setEffectProg(GLuint program){
   effect.setShaderProg(program);
}

void Entity::update() {
	static int frames = 0;
	static float fps = 1 / 60.f;
	
	float dt = 6.f * 1.f / fps; // fix time step according to the fps
	
	float dot;
	glm::vec3 vn;
	glm::vec3 fd;
	
	// If this is an AI entity, update it's state
	if (type == AI_ENTITY || type == PLAYER_ENTITY)
		ai_->update(this);
	
	// Obtain the angle between the two quats, use this for proportional control of craft
	dot = glm::dot(rotation, target_rotation);
	dot = glm::abs(dot) > 20.f ? 20.f : glm::abs(dot);
	
	// Control loop the rotation to the desired rotation
	rotation = glm::shortMix(rotation, target_rotation, glm::abs(dot) / 20.f);
	
	// Update the position by moving velocity in direction
	glm::vec3 tempPos = (position.y >= 0.f) ? rotation * velocity * dt : glm::vec3(0, 0.0001f, 0);
	if (tempPos.y + position.y <= 45) position += tempPos;
	vn = glm::vec3(0, 0, 1);
	fd = -0.5f * glm::vec3(0, 0, -1) * 1.293f * drag * carea * velocity * velocity;
	force = thrust * vn * mass;
	velocity += (force + fd) * (1.f / mass) * dt;
	
	frames++;
	fps = frames / glfwGetTime();
}

void Entity::update(glm::mat4 viewMat, glm::mat4 projMat) {
	flame.update(viewMat, projMat, rotation, position);
}

void Entity::updateEffect(glm::mat4 viewMat, glm::mat4 projMat) {
   effect.update(viewMat, projMat, rotation, position);
}

void Entity::drawExhaust() {
   flame.draw(thrust);
}

void Entity::drawEffect() {
   effect.draw(thrust);
}

void Entity::throttleUp() {
	static int set = 0;
	float mod = 0.1f;
	
	// limit the maximum thrust
	thrust -= (thrust > max_thrust ? mod : 0.f);
}

void Entity::throttleDown() {
	// limit the minimum thrust
	thrust += (thrust < 0 ? 0.1f : 0.f);
}

void Entity::pitch(float dy) {
	// Limit the pitch angle
	dy = dy > 50.f ? 50.f : dy;
	dy = dy < -50.f ? -50.f : dy;
	
	pitch_angle = dy / 360.f;
	
	// Build dy pitch rotation glm::quat around x axis
	glm::quat rot = glm::angleAxis(pitch_angle, glm::vec3(1, 0, 0));
	
	// Apply pitch change to the current rotation.
	target_rotation *= rot;
}

void Entity::yaw(float dx) {
	// Limit the yaw angle
	dx = dx > 50.f ? 50.f : dx;
	dx = dx < -50.f ? -50.f : dx;
	
	// Build dx yaw rotation glm::quat around y axis
	glm::quat rot = glm::angleAxis(dx / 360.f, glm::vec3(0, 1, 0));
	
	// Apply yaw change to the current rotation
	target_rotation *= rot;
}

void Entity::rollRight() {
	// build roll quat
	glm::quat rol = glm::angleAxis(-0.15f, glm::vec3(0, 0, 1));
	
	// Apply roll change
	target_rotation *= rol;
}

void Entity::rollLeft() {
	// build roll quat
	glm::quat rol = glm::angleAxis(0.15f, glm::vec3(0, 0, 1));
	
	// Apply roll change
	target_rotation *= rol;
}

void Entity::turn(float dx) {
	// Limit the turn angle
	dx = dx > 50.f ? 50.f : dx;
	dx = dx < -50.f ? -50.f : dx;
	
	// Build dx yaw rotation glm::quat around y axis
	glm::quat rot = glm::angleAxis(-dx / 360.f, glm::vec3(0, 1, 0));
	
	// Build dx roll rotation glm::quat around z axis
	glm::quat rol = glm::angleAxis(-dx / 360.f, glm::vec3(0, 0, 1));
	
	// Apply yaw change to the current rotation.
	target_rotation *= glm::mix(rot, rol, 0.6f);
}

void Entity::packVertices(vector<float> *pbo, vector<float> *nbo, vector<unsigned int> *ibo, std::vector<float> *tbo = NULL) {
	int iboIdx = 0;
	for (size_t i=0; i < object->shapes.size(); i++) {
		pbo->insert(pbo->end(), object->shapes[i].mesh.positions.begin(), object->shapes[i].mesh.positions.end());
		nbo->insert(nbo->end(), object->shapes[i].mesh.normals.begin(), object->shapes[i].mesh.normals.end());
		
		if(tbo)
			tbo->insert(tbo->end(), object->shapes[i].mesh.texcoords.begin(), object->shapes[i].mesh.texcoords.end());
		
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

Material Entity::getBaseMaterial() {
	return base_material;
}

glm::vec3 Entity::getPosition() {
	return position;
}

float Entity::getPitch() {
	return pitch_angle;
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

glm::vec3 Entity::getDirection() {
	return glm::normalize(glm::vec3(0, 0, -1) * rotation);
}

float Entity::getThrust() {
	return thrust;
}

AIComponent * Entity::getAI() {
	return ai_;
}

void Entity::setObject(Object *obj) {
	object = obj;
}

void Entity::setPosition(glm::vec3 pos) {
	position = glm::vec3(pos);
}

void Entity::setScale(glm::vec3 sc) {
	scale = glm::vec3(sc);
}

void Entity::setTargetRotationQ(glm::quat r) {
	target_rotation = r;
}

void Entity::setRotationQ(glm::quat r) {
	rotation = r;
}

void Entity::setVelocity(glm::vec3 vel) {
	velocity = glm::vec3(vel);
}

void Entity::setThrust(float th) {
	thrust = th;
}

void Entity::setMaxThrust(float th) {
	thrust = th;
}

void Entity::setMaterial(Material mat) {
	material = mat;
}

void Entity::setBaseMaterial(Material mat) {
	base_material = mat;
}

void Entity::setFlag(EntityFlag f) {
	flag = f;
}

void Entity::setType(EntityType new_type) {
	type = new_type;
}

EntityFlag Entity::getFlag() {
	return flag;
}

float Entity::getRadius() {
	return radius;
}

void Entity::calculateBoundingSphereRadius() {
	
	glm::vec3 center = glm::vec3(object->shapes[0].mesh.positions[0], object->shapes[0].mesh.positions[1], object->shapes[0].mesh.positions[2]);
	float calculatedRadius = FLT_MIN;
	glm::vec3 pos, diff;
	float length, alpha, alphaSq;
	
	for (int i = 0; i < 2; i++){
		for (int vertex = 0; vertex < object->shapes[0].mesh.indices.size()/3; vertex++) {
			int index = object->shapes[0].mesh.indices[3 * vertex];
			pos = glm::vec3(object->shapes[0].mesh.positions[3 * index + 0], object->shapes[0].mesh.positions[3 * index + 1], object->shapes[0].mesh.positions[3 * index + 2]);
			length = glm::distance(pos, center);
			if (length > calculatedRadius) {
				alpha = length / calculatedRadius;
				alphaSq = alpha * alpha;
				calculatedRadius = 0.5f * (alpha + 1 / alpha) * calculatedRadius;
				center = 0.5f * ((1 + 1 / alphaSq) * center + (1 - 1 / alphaSq) * pos);
			}
		}
	}
	
	for (int vertex = 0; vertex < object->shapes[0].mesh.indices.size()/3; vertex++) {
		int index = object->shapes[0].mesh.indices[3 * vertex];
		pos = glm::vec3(object->shapes[0].mesh.positions[3 * index + 0], object->shapes[0].mesh.positions[3 * index + 1], object->shapes[0].mesh.positions[3 * index + 2]);
		diff = pos - center;
		length = glm::distance(pos, center);
		if (length > calculatedRadius){
			calculatedRadius = (calculatedRadius + length) / 2.0f;
			center = center + ((length - calculatedRadius) / length * diff);
		}
	}
	
	radius = calculatedRadius * scale.x;
}

void Entity::addAmmo(int amount) {
	ammunition += amount;
}

int Entity::getAmmo() {
	return ammunition;
}

void Entity::subtractAmmo(int amount) {
	if (ammunition - amount < 0) ammunition = 0;
	else ammunition -= amount;
}

#endif
