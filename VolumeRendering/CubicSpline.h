#pragma once
#include "cinder/CinderGlm.h"
#include <vector>

class CubicSpline
{
    public:
        CubicSpline() = default;
        CubicSpline(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d);
        //evaluate the point using a cubic equation
        glm::vec3 getPointOnSpline(float s) const;
        static std::vector<CubicSpline> CalculateCubicSpline(std::vector<glm::vec3> points);
    private:
        glm::vec3 a;
        glm::vec3 b;
        glm::vec3 c;
        glm::vec3 d;
};