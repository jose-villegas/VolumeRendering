#pragma once
#include "cinder/CinderMath.h"

struct Light
{
    glm::vec3 direction{ 0.0f, 0.0f, 1.0f };
    glm::vec3 diffuse{ 1.0f, 1.0f, 1.0f };
    glm::vec3 ambient{ 0.1f, 0.1f, 0.1f };
};

