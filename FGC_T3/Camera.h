#pragma once
#include "GL\glm\gtc\matrix_transform.hpp"

class Camera {
    public:
        static float camSpeed;
        static glm::vec3 direction; // For FPS Mode
        static glm::vec3 position;
        static glm::vec3 lookAtPosition;
        static glm::vec3 upVector;
        static glm::mat4 getViewMatrix();
};
