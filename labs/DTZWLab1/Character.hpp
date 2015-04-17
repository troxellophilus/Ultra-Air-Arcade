/*
 * Defines a character in the game world.
 */

#ifndef CHARACTER_H
#define CHARACTER_H

#include <glm/glm.hpp>
#include "helper.h"

class Character {
private:
    Object *obj;   // obj vertices
    Object *ind;
    glm::vec3 pos; // translation transform for current position
    glm::vec3 ipos;
    glm::vec3 vel; // vx, vy, vz of the character in units/s
    glm::vec3 ori;
    uint32_t mat;
    uint32_t imat;
    bool cflag;
    bool bflag;
    bool uflag;
    bool rflag;
    float idir;
    
public:
    Character(Object *, Object *, glm::vec3, glm::vec3, uint32_t, uint32_t);
    Object * getObject();
    Object * getIndicator();
    glm::vec3 getPosition();
    glm::vec3 getIPosition();
    glm::vec3 getVelocity();
    glm::vec3 getOrientation();
    uint32_t getMaterial();
    uint32_t getIMaterial();
    void setPosition(glm::vec3);
    void setObject(Object *);
    void setVelocity(glm::vec3);
    void setMaterial(uint32_t material);
    void setIMaterial(uint32_t imaterial);
    void setOrientation(glm::vec3);
    void setIndicator(Object *obj);
    void setCFlag();
    bool getCFlag();
    void setBFlag(bool);
    bool getBFlag();
    void setUFlag();
    bool getUFlag();
    void update(double);
};

Character::Character(Object *object, Object *indicator, glm::vec3 position, glm::vec3 velocity, uint32_t material, uint32_t imaterial) {
    setObject(object);
    setIndicator(indicator);
    setPosition(position);
    setVelocity(velocity);
    setMaterial(material);
    setIMaterial(imaterial);
    setOrientation(glm::vec3(0, 5, 0));
    cflag = false;
    bflag = false;
    uflag = false;
    rflag = false;
    idir = 0.001f;
    
    ipos = glm::vec3(position);
    ipos.y += 0.4f;
}

void Character::update(double elapsed) {
    if (cflag) {
        setIMaterial(0);
        setVelocity(glm::vec3(0));
    }
    else if (bflag && !rflag) {
        setVelocity(glm::vec3(0) - vel);
        rflag = true;
    }
    else {
        if (rflag && uflag) {
            bflag = false;
            uflag = false;
            rflag = false;
        }
        pos.x += (vel.x ? (vel.x / elapsed) : 0);
        pos.y += (vel.y ? (vel.y / elapsed) : 0);
        pos.z += (vel.z ? (vel.z / elapsed) : 0);
        
        ipos.x = pos.x;
        ipos.z = pos.z;
        
        if (ipos.y > (pos.y + 0.3) || ipos.y < (pos.y + 0.1))
            idir = -idir;
        ipos.y += idir;

	ori.y += 0.25;
    }
}

Object * Character::getObject() {
    return obj;
}

Object * Character::getIndicator() {
    return ind;
}

glm::vec3 Character::getPosition() {
    return pos;
}

glm::vec3 Character::getIPosition() {
    return ipos;
}

glm::vec3 Character::getVelocity() {
    return vel;
}

glm::vec3 Character::getOrientation() {
    return ori;
}

uint32_t Character::getMaterial() {
    return mat;
}

uint32_t Character::getIMaterial() {
    return imat;
}

void Character::setObject(Object *object) {
    obj = object;
}

void Character::setIndicator(Object *indicator) {
    ind = indicator;
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

void Character::setMaterial(uint32_t material) {
    mat = material;
}

void Character::setIMaterial(uint32_t imaterial) {
    imat = imaterial;
}

void Character::setCFlag() {
    cflag = true;
}

bool Character::getCFlag() {
    return cflag;
}

void Character::setBFlag(bool val) {
    bflag = val;
}

bool Character::getBFlag() {
    return bflag;
}

bool Character::getUFlag() {
    return uflag;
}

void Character::setUFlag() {
    uflag = true;
}

#endif
