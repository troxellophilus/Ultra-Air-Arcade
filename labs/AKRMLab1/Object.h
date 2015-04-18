#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cassert>
#include <cmath>
#include <stdio.h>
#include "GLSL.h"
#include "tiny_obj_loader.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr
#include "RenderingHelper.h"

#include "tiny_obj_loader.h"
#include "AABB.h"

using namespace glm;
using namespace std;
using namespace tinyobj;

class Object {
public:
   //(x,y,z) position, e.g., it's center point
   vec3 position;
   //(x,y,z) direction vector (y direction component must be zero)
   vec3 direction;
   //a scalar velocity  (or if desired you may store ‘velocity’ in the direction vector)
   float velocity;
   //an axis aligned bounding box (or AABB): min x,y,z and max x,y,z
   AABB boundingBox;
   //whether or not the object has been touched by the camer
   bool touched;

   vector<shape_t> shape;
   vector<material_t> material;

   //Buffers
   GLuint posBufObj;
   GLuint norBufObj;
   GLuint indBufObj;

   //initialize the position, direction, velocity, and bounding box 
   Object();
   Object(vec3 pos, vec3 dir, float vel, AABB bb);
   virtual ~Object();
   void resize_obj();
   void loadShapes(const string &objFile);
   vector<float> getNorBuf();
   void initShape();
   void timeStep(float dt);
   void setObject(RenderingHelper ModelTrans);
   void draw(GLint h_aPosition, GLint h_aNormal, GLint h_uModelMatrix, RenderingHelper ModelTrans);
};