/*
 * Class to define and implement a Plane Sound component
 * Drew Troxell
 */

#ifndef PLANE_SOUND_H
#define PLANE_SOUND_H

#include "Entity.hpp"

class PlaneSound {
private:

public:
    // Constructors
    PlaneSound();
    
    // Methods
    void update(Entity *agent);
};

// Constructors
PlaneSound::PlaneSound() {
}

// Methods
void PlaneSound::update(Entity *e, Entity *p) {
    static unsigned int frames = 0;

    // Get entity info and play sounds accordingly for planes 

    frames++;
}

// Getters

// Setters

#endif
