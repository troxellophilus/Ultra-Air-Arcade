/*
 * Class to define a camera in 3D space
 * Drew Troxell
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#define PI 3.14159265

#define FLAG_RESET_CAM 0x01

// Default values for Camera (uses these if not set to anything else)
#define FOVY           75.0
#define ZNEAR           0.1
#define ZFAR         1000.0
#define SCROLL_SPEED    0.001
#define CAM_SPEED      30.0

class Camera {
private:
	// Time
	float lst; // Last time
	float cur; // Current time

	// Cursor
	float cpx; // Cursor position x
	float cpy; // Cursor position y

	// Window
	float wdt; // Width of the window
	float hgt; // Height of the window

	// Perspective
	float fov; // Field of view y
	float asp; // Aspect ratio
	float znr; // Z Near
	float zfr; // Z Far

	// View
	glm::vec2 rot; // Rotations
	float rfa; // rfactor
	float tfa; // tfactor
	glm::vec3 eye; // Position of the camera
	glm::vec3 dir; // Camera position + direction vector
	
	// Flags
	uint8_t flg = 0x0;

public:
	// Constructors
	Camera(float start, float width, float height);

	// Getters
	glm::mat4 getProjectionMatrix();
	glm::mat4 getViewMatrix();
	glm::vec3 getPosition();
	glm::vec3 getDirection();
	uint8_t getFlags();
	
	// Setters
	void setFovy(float fovy);
	void setNearZ(float zNear);
	void setFarZ(float zFar);
	void setPosition(glm::vec3 position);
	void setDirection(glm::vec3 direction);
	void setUpAngle(glm::vec3 upAngle);
	
	// Methods
	void update(float time, bool *keys, float cursorX, float cursorY);
};

// Constructors

Camera::Camera(float start, float width, float height) {
	wdt = width;
	hgt = height;
	cpx = 0;
	cpy = 0;
	rot = glm::vec2(0.,-PI);
	fov = FOVY;
	asp = width / height;
	znr = ZNEAR;
	zfr = ZFAR;
	rfa = SCROLL_SPEED;
	tfa = CAM_SPEED;
	eye = glm::vec3(0.f, 0.4f, 0.f);
	lst = start;
	dir = glm::vec3(0.f,0.f,-1.f);
}

// Methods

void Camera::update(float time, bool *keys, float cursorX, float cursorY) {
	cur = time;
	float tDif = cur - lst;

	glm::vec2 dv = glm::vec2(cursorX - cpx, cursorY - cpy);

	rot -= rfa * dv;

	glm::mat4 R = glm::rotate(rot.x, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(rot.y, glm::vec3(1.0f,0.0f,0.0f));
	glm::vec3 RZ = glm::vec3(R[2][0], R[2][1], R[2][2]);
	glm::vec3 RX = glm::vec3(R[0][0], R[0][1], R[0][2]);

	dir = RX;

	if (keys['W'])
		eye -= RZ * tDif * tfa;
	if (keys['S'])
		eye += RZ * tDif * tfa;
	if (keys['D'])
		eye -= RX * tDif * tfa;
	if (keys['A'])
		eye += RX * tDif * tfa;

	lst = cur;

	cpx = cursorX;
	cpy = cursorY;
}

// Getters

glm::mat4 Camera::getProjectionMatrix() {
	return glm::perspective(fov, asp, znr, zfr);
}

glm::mat4 Camera::getViewMatrix() {
	glm::mat4 T = glm::translate(eye);
	glm::mat4 R = glm::rotate(rot.x, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(rot.y, glm::vec3(1.0f,0.0f,0.0f));
	
	glm::mat4 C = T * R;

	return glm::inverse(C);
}

glm::vec3 Camera::getPosition() {
	return glm::vec3(eye);
}

glm::vec3 Camera::getDirection() {
	return glm::vec3(dir);
}

uint8_t Camera::getFlags() {
	return flg;
}

// Setters

void Camera::setFovy(float fovy) {
	fov = fovy;
}

void Camera::setNearZ(float zNear) {
	znr = zNear;
}

void Camera::setFarZ(float zFar) {
	zfr = zFar;
}

void Camera::setPosition(glm::vec3 position) {
	eye = glm::vec3(position);
}

void Camera::setDirection(glm::vec3 direction) {
	dir = glm::vec3(direction);
}

#endif
