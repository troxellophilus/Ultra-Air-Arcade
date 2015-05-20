#ifndef FRUSTUM_H
#define FRUSTUM_H

#define GLM_FORCE_RADIANS

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtc/type_ptr.hpp"

// #include <glm/glm.hpp>
// #include <glm/gtx/transform.hpp>
// #include <glm/gtx/rotate_vector.hpp>
// #include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <string>
#include <iostream>
#include <vector>
#include <stdio.h>

#include "PlaneData.h"

#define NUM_PLANES 6



class Frustum {
public:
	PlaneData viewFrustum[NUM_PLANES];

	//Constructor/destructor
	Frustum();
	virtual ~Frustum();

	//Functions
	void normalize(int side);
	void setFrustum(glm::mat4 viewMatrix, glm::mat4 projMatrix);
	bool sphereInFrustum(glm::vec3 pos, float r);
};

#endif