#include "Camera.h"

//Helper class to represent the camera

Camera::Camera(vec3 pos, float t, float p) {
   position = pos;
   theta = t;
   phi = p;
}

Camera::~Camera() {
   // TODO Auto-generated destructor stub
}

//Set the position of the camera
void Camera::setPosition(vec3 pos) {
   position = pos;
}

//Set horizontal angle
void Camera::setTheta(float t) {
   theta = t;
}

//Set vertical angle
void Camera::setPhi(float p) {
   phi = p;
}

//Return camera position
vec3 Camera::getPosition() {
   return position;
}

//Return horizontal angle
float Camera::getTheta() {
   return theta;
}

//Return vertical angle
float Camera::getPhi() {
   return phi;
}

//Return the x dimension
void Camera::setY(float y) {
   position.y = y;
}

//Return the y dimension
float Camera::getY() {
   return position.y;
}