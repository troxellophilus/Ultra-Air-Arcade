#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cassert>
#include <cmath>
#include <stdio.h>
#include <glm/glm.hpp>
#include "types.h"

using namespace std;

#ifndef HELPER_H
#define HELPER_H

bool installShaders(const std::string &vShaderName, const std::string &fShaderName);
float randNum();
float randAngle();
void calcTime();
void loadShapes(const std::string &objFile, Object &obj);
void initGL(int i);
void moveView();
void drawGL(int i);
void initVars();
// resize all vertices to the range [-1, 1]
void resize_obj(std::vector<tinyobj::shape_t> &shapes);
void initGround();

#endif
