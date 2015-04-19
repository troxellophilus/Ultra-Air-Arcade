#ifndef TYPES_H
#define TYPES_H

#include "tiny_obj_loader.h"

#define NUMSHAPES 3

typedef struct Object {
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::vector<float> normals;
} Object;

#endif
