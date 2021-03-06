#include "AABB.h"

AABB::AABB() {

}

AABB::AABB(vec3 c, vec3 r) {
   center = c;
   radius = r;
}

AABB::~AABB() {
   // TODO Auto-generated destructor stub
}

bool AABB::testCollision(AABB a) {
   bool xOverlap = true;
   bool yOverlap = true;
   bool zOverlap = true;
   bool anyOverlap = false;

   if (fabs(a.center.x - center.x) > (a.radius.x + radius.x)) 
      xOverlap = false;
   if (fabs(a.center.y - center.y) > (a.radius.y + radius.y)) 
      yOverlap = false;
   if (fabs(a.center.z - center.z) > (a.radius.z + radius.z)) 
      zOverlap = false;

   //cout << xOverlap << " " << yOverlap << " " << zOverlap << endl;

   anyOverlap = xOverlap && yOverlap && zOverlap;

   return anyOverlap;
}