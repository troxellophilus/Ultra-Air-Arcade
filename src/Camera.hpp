/*
 * Class to define a camera in 3D space
 * Drew Troxell
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Entity.hpp"

#define PI 3.14159265

enum CameraMode { TPC, FREE, SPLASH };

enum CameraDirection { FORWARD, BACK, LEFT, RIGHT, UP, DOWN };

class Camera {
private:
    // Mode
    CameraMode mode; // mode of the camera
    
    // Window
    float window_width; // Width of the window
    float window_height; // Height of the window
    float viewport_x;
    float viewport_y;
    float aspect;
    
    // Perspective
    float field_of_view; // Field of view y
    float aspect_ratio; // Aspect ratio
    float z_near; // Z Near
    float z_far; // Z Far
    
    // View
    glm::quat rotation; // Rotation of the camera as a glm::quaternion
    glm::vec3 position; // Position of the camera
    
    // Player
    Entity *player;
    
public:
    // Constructors
    Camera();
    
    // Getters
    glm::mat4 getProjectionMatrix();
    glm::mat4 getViewMatrix();
    glm::vec3 getPosition();
    CameraMode getMode();
    
    // Setters
    void setFOV(float fov);
    void setClipping(float zNear, float zFar);
    void setPosition(glm::vec3 position);
    void setPlayer(Entity *player);
    void setMode(CameraMode m);
    
    // Methods
    void update();
    void move(CameraDirection dir);
    void pitch(float dy);
    void yaw(float dx);
};

// Constructors
Camera::Camera() {
    // Initialize camera mode
    mode = TPC;
    
    // Initialize viewport
    window_width = 640;
    window_height = 480;
    viewport_x = 0;
    viewport_y = 0;
    
    // Initialize perspective
    field_of_view = 75;
    aspect = window_width / window_height;
    z_near = 0.01;
    z_far = 2000;
    
    // Initialize camera transforms
    position = glm::vec3(0, 0, 0);
    rotation = glm::quat(1, 0, 0, 0);
}

// Methods

void Camera::update() {
    static glm::quat last = player->getRotationQ();
    float mixFact = 1;
    
    if (mode == TPC) {
        position = player->getPosition();
        
        if (player->getRotationQ() != last)
            mixFact = 0.1f;
        
        rotation = glm::mix(rotation, player->getRotationQ(), mixFact);
        
        last = player->getRotationQ();
    }
    else if (mode == SPLASH) {
        //position = glm::vec3(0, 0, 0);
    }
}

void Camera::move(CameraDirection dir) {
    if (mode == FREE) {
        glm::mat4 RM = glm::toMat4(rotation);
        glm::vec3 RZ = glm::vec3(RM[2][0], RM[2][1], RM[2][2]);
        glm::vec3 RX = glm::vec3(RM[0][0], RM[0][1], RM[0][2]);
        glm::vec3 RY = glm::vec3(RM[1][0], RM[1][1], RM[1][2]);
        
        float spd = 1.0f;
        float dt = 1 / 60.f;
        
        switch (dir) {
            case FORWARD:
                position -= spd * RZ * dt;
                break;
            case BACK:
                position += spd * RZ * dt;
                break;
            case LEFT:
                position -= spd * RX * dt;
                break;
            case RIGHT:
                position += spd * RX * dt;
                break;
            case UP:
                position -= spd * RY * dt;
                break;
            case DOWN:
                position += spd * RY * dt;
                break;
        }
    }
}

void Camera::pitch(float dy) {
    if (mode == FREE) {
        // Build dy pitch rotation quat around x axis
        glm::quat rot = glm::angleAxis(dy / 360.f, glm::vec3(1, 0, 0));
        
        // Apply pitch change to the current rotation
        rotation *= rot;
    }
}

void Camera::yaw(float dx) {
    if (mode == FREE) {
        // Build dx yaw rotation quat around y axis
        glm::quat rot = glm::angleAxis(dx / 360.f, glm::vec3(0, 1, 0));
        
        // Apply yaw change to the current rotation
        rotation *= rot;
    }
}

// Getters

glm::mat4 Camera::getProjectionMatrix() {
    return glm::perspective(field_of_view, aspect, z_near, z_far);
}

glm::mat4 Camera::getViewMatrix() {
    glm::mat4 T = glm::translate(position);
    glm::mat4 R = glm::toMat4(rotation);
    glm::mat4 TP = glm::translate(glm::vec3(0, 0.2, 0.2));
    
    glm::mat4 C = T * R * TP;
    
    return glm::inverse(C);
}

glm::vec3 Camera::getPosition() {
    return glm::vec3(position);
}

CameraMode Camera::getMode() {
    return mode;
}

// Setters

void Camera::setFOV(float fov) {
    field_of_view = fov;
}

void Camera::setClipping(float zNear, float zFar) {
    z_near = zNear;
    z_far = zFar;
}

void Camera::setPosition(glm::vec3 pos) {
    position = glm::vec3(pos);
}

void Camera::setPlayer(Entity *entity) {
    player = entity;
}

void Camera::setMode(CameraMode m) {
    mode = m;
}

#endif
