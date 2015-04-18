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

using namespace glm;
using namespace std;

class AABB {
public:
   vec3 center;
   vec3 radius;

   AABB();
   AABB(vec3 c, vec3 r);
   virtual ~AABB();
   bool testCollision(AABB a);
};