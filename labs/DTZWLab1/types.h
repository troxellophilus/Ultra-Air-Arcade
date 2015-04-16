#include <stdlib.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //perspective, trans etc
#include <glm/gtc/type_ptr.hpp> //value_ptr
#include "GLSL.h"
#include "tiny_obj_loader.h"
#include "shader.hpp"
#include "text2D.hpp"
#include "texture.hpp"

#ifndef TYPES_H
#define TYPES_H
#define NUMSHAPES 3
#ifndef __APPLE__
	#define SCROLLSPEED 0.005f
#else
	#define SCROLLSPEED 0.00005f
#endif
#define CAMSPEED 2.0f

typedef struct Object {
   std::vector<tinyobj::shape_t> shapes;
   std::vector<tinyobj::material_t> materials;
   std::vector<float> normals;
} Object;

#endif
