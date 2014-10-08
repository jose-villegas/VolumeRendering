#include "Camera.h"

glm::vec3 Camera::position = glm::vec3(0.0f, 0.0f, 1.5f);

glm::vec3 Camera::lookAtPosition = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 Camera::direction = glm::vec3(0.0f, 0.0f, -1.0f);

glm::vec3 Camera::upVector = glm::vec3(0.0f, 1.0f, 0.0f);

float Camera::camSpeed = 10;

glm::mat4 Camera::getViewMatrix()
{
    //////////////////////////////////////////////////////////////////////////
    // FPS CAMERA MODE
    lookAtPosition = position + direction;
    //////////////////////////////////////////////////////////////////////////
    return glm::lookAt(position, lookAtPosition, upVector);
}


