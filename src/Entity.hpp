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

using namespace std;

class Entity {
private:
    // Object shape & color properties
    Object    *object;  // obj vertices
    Material  material; // Material of the entity
    glm::vec3 scale;    // scale of the object model
    
    // Position & orientation properties
    glm::vec3 position;        // translation transform for current position
    glm::vec3 direction;       // Direction vector of the entity
    glm::quat rotation;        // Quaternion rotation of entity
    glm::quat target_rotation; // Quaternion target rotation of entity
    
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

    // Bounding sphere radius
    float radius;
    float calculateBoundingSphereRadius();
    
public:
    // Constructor
    Entity();
    
    // Getters
    Object * getObject();
    Material getMaterial();
    
    glm::vec3 getPosition();
    glm::vec3 getScale();
    glm::mat4 getRotationM();
    glm::quat getRotationQ();
    
    glm::vec3 getVelocity();
    float getThrust();
    
    EntityFlag getFlag();

    float getRadius();
    
    // Setters
    void setObject(Object *);
    void setMaterial(Material material);
    
    void setPosition(glm::vec3);
    void setScale(glm::vec3);
    
    void setVelocity(glm::vec3);
    void setThrust(float a);
    
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
    rotation = glm::quat(1, 0, 0, 0);
    target_rotation = glm::quat(1, 0, 0, 0);
    
    scale = glm::vec3(1, 1, 1);
    mass = 2000.0f;
    force = glm::vec3(0, 0, 0);
    drag = 0.47f; // sphere for now
    carea = 20.f;
    
    thrust = 0.f;
    velocity = glm::vec3(0, 0, 0);
    
    flag = C_FLAG;

    radius = calculateBoundingSphereRadius();
}

void Entity::update() {
    float dt = 1 / 60.f; // fixed time step
    float dot;
    glm::vec3 vn;
    glm::vec3 fd;
    
    // Obtain the angle between the two quats, use this for proportional control of craft
    dot = glm::dot(glm::normalize(rotation), glm::normalize(target_rotation));
    dot = glm::abs(dot) > 30.f ? 30.f : glm::abs(dot);
    
    // Control loop the rotation to the desired rotation
    rotation = glm::mix(rotation, target_rotation, dot / 30.f);
    
    // Update the position by moving velocity in direction
    position += (position.y >= 0.f) ? rotation * velocity * dt : glm::vec3(0, 0, 0);
    vn = glm::vec3(0, 0, 1);
    fd = -0.5f * glm::vec3(0, 0, -1) * 1.293f * drag * carea * velocity * velocity;
    force = thrust * vn * mass;
    velocity += (force + fd) * (1.f / mass) * dt;
}

void Entity::throttleUp() {
    // limit the maximum thrust
    thrust -= thrust > -5.f ? 0.1f : 0.f;
}

void Entity::throttleDown() {
    // limit the minimum thrust
    thrust += thrust < 0 ? 0.1f : 0.f;
}

void Entity::pitch(float dy) {
    // Limit the pitch angle
    dy = dy > 50.f ? 50.f : dy;
    dy = dy < -50.f ? -50.f : dy;
    
    // Build dy pitch rotation glm::quat around x axis
    glm::quat rot = glm::angleAxis(dy / 360.f, glm::vec3(1, 0, 0));
    
    // Apply pitch change to the current rotation.
    target_rotation *= rot;
}

void Entity::rollRight() {
    // build roll quat
    glm::quat rol = glm::angleAxis(-0.2f, glm::vec3(0, 0, 1));
    
    // Apply roll change
    target_rotation *= rol;
}

void Entity::rollLeft() {
    // build roll quat
    glm::quat rol = glm::angleAxis(0.2f, glm::vec3(0, 0, 1));
    
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
    target_rotation *= glm::mix(rot, rol, 0.8f);
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

float Entity::getRadius() {
    return radius;
}

float Entity::calculateBoundingSphereRadius() {
    return 0.5f;

    // TODO add algorithm for determining bounding sphere radius
}

#endif
