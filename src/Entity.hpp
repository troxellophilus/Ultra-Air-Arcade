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
    glm::vec3 velocity; // x, y, z velocity of the entity u/s
    
    // Flags if necessary
    EntityFlag flag; // Entity flags, use as needed
    EntityType type; // Flag to designate the type of entity

    // Bounding sphere radius
    float radius;
    
public:
    // Constructor
    Entity(AIComponent *ai);
    Entity();
    
    // Getters
    Object * getObject();
    Material getMaterial();
    
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
    
    void setPosition(glm::vec3);
    void setScale(glm::vec3);
    void setTargetRotationQ(glm::quat);
    
    void setVelocity(glm::vec3);
    void setThrust(float a);
    
    void setFlag(EntityFlag new_flag);
    void setType(EntityType new_type);
    
    // Methods
    void update();
    
    void pitch(float dy);
    void yaw(float dx);
    void turn(float dx);
    void rollRight();
    void rollLeft();
    
    void throttleUp();
    void throttleDown();
    
    void packVertices(vector<float> *, vector<float> *, vector<unsigned int> *);

    void calculateBoundingSphereRadius();
};

Entity::Entity() {
    object = NULL;
	// Zach - removed Material:: from the call to the Material constructor - 
	// Would not allow my version to compile
    material = Material();
    
    position = glm::vec3(0, 0, 0);
    rotation = glm::quat(1, 0, 0, 0);
    target_rotation = glm::quat(1, 0, 0, 0);
    
    
    scale = glm::vec3(1, 1, 1);
    mass = 5000.0f;
    force = glm::vec3(0, 0, 0);
    drag = 1.5f; // sphere for now
    carea = 25.f;
    
    thrust = 0.f;
    velocity = glm::vec3(0, 0, 0);
    
    flag = C_FLAG;
    type = AI_ENTITY;

    radius = 0.f;

    ai_ = NULL;
}

Entity::Entity(AIComponent *ai) {
    object = NULL;
	// Zach - removed Material:: from the call to the Material constructor - 
	// Would not allow my version to compile
    material = Material();
    
    position = glm::vec3(0, 0, 0);
    rotation = glm::quat(1, 0, 0, 0);
    target_rotation = glm::quat(1, 0, 0, 0);
    
    scale = glm::vec3(1, 1, 1);
    mass = 5000.0f;
    force = glm::vec3(0, 0, 0);
    drag = 1.5f; // sphere for now
    carea = 25.f;
    
    thrust = 0.f;
    velocity = glm::vec3(0, 0, 0);
    
    flag = C_FLAG;
    type = AI_ENTITY;

    radius = 0.f;

    ai_ = ai;
}

void Entity::update() {
    float dt = 1 / 60.f; // fixed time step
    float dot;
    glm::vec3 vn;
    glm::vec3 fd;

    // If this is an AI entity, update it's state
    if (type == AI_ENTITY)
        ai_->update(this);
    
    // Obtain the angle between the two quats, use this for proportional control of craft
    dot = glm::dot(rotation, target_rotation);
    dot = glm::abs(dot) > 30.f ? 30.f : glm::abs(dot);
    
    // Control loop the rotation to the desired rotation
    rotation = glm::shortMix(rotation, target_rotation, glm::abs(dot) / 30.f);
    
    // Update the position by moving velocity in direction
    position += (position.y >= 0.f) ? rotation * velocity * dt : glm::vec3(0, 0.0001f, 0);
    vn = glm::vec3(0, 0, 1);
    fd = -0.5f * glm::vec3(0, 0, -1) * 1.293f * drag * carea * velocity * velocity;
    force = thrust * vn * mass;
    velocity += (force + fd) * (1.f / mass) * dt;
}

void Entity::throttleUp() {
    float top_speed = -1.f;
    float mod = 0.f;
    if (type == AI_ENTITY) {
	top_speed = -1.f;
	mod = 0.01f;
    }
    if (type == PLAYER_ENTITY) {
	top_speed = -1.1f;
	mod = 0.1f;
    }
    // limit the maximum thrust
    thrust -= thrust > -1.f ? mod : 0.f;
}

void Entity::throttleDown() {
    // limit the minimum thrust
    thrust += thrust < 0 ? 0.1f : 0.f;
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
    target_rotation *= glm::mix(rot, rol, 0.3f);
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

void Entity::setVelocity(glm::vec3 vel) {
    velocity = glm::vec3(vel);
}

void Entity::setThrust(float th) {
    thrust = th;
}

void Entity::setMaterial(Material mat) {
    material = mat;
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
    //radius = 0.05f;

    glm::vec3 center = glm::vec3(object->shapes[0].mesh.positions[0], object->shapes[0].mesh.positions[1], object->shapes[0].mesh.positions[2]);
    float calculatedRadius = 0.000000000001f;
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

    // printf("Radius: %.4f\n", calculatedRadius);
    // printf("Scaled Radius: %.4f\n", calculatedRadius * scale.x);

    radius = calculatedRadius * scale.x;
}

#endif
