#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cassert>
#include <cmath>
#include <stdio.h>
#include "GLSL.h"
#include "glm/glm.hpp"

using namespace glm;
using namespace std;

class BoundingSphere {
public:
   vec3 center;
   vec3 radius;

   BoundingSphere();
   BoundingSphere(vec3 c, vec3 r);
   virtual ~BoundingSphere();
   bool testCollision(BoundingSphere a);
};
