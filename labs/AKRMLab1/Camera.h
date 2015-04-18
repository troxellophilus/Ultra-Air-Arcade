#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr

using namespace glm;
using namespace std;

//Helper class to represent the camera
class Camera {
public:
   //Position of the camera in space
   vec3 position;
   //Represent the horizontal angle
   float theta;
   //Represent the vertical angle
   float phi;

   Camera(vec3 pos, float t, float p);
   virtual ~Camera();
   void setPosition(vec3 pos);
   void setTheta(float t);
   void setPhi(float p);
   vec3 getPosition();
   float getTheta();
   float getPhi();
   void setY(float y);
   float getY();
};