#pragma once
#include "TransferFunctionPoint.h"
class RaycastVolume;

class TransferFunction
{
public:
    TransferFunction();
    ~TransferFunction();
    glm::vec4 getColor(const float t /* t=[0..1] */);
    glm::vec4 getColor(const int isoValue /* isoValue=[0..255] */);
    void updateFunction();
    void addColorPoint(const glm::vec3& color, const int isoValue);
    void addAlphaPoint(const float alpha, const int isoValue);
    void removeColorPoint(const int isoValue);
    void removeAlphaPoint(const int isoValue);
    void setThreshold(int minIso, int maxIso);
    const glm::ivec2 &getThreshold() const;
    const cinder::gl::Texture1dRef &get1DTexture() const;
protected:
    class Cubic
    {
    public:
        Cubic() = default;
        Cubic(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d);
        //evaluate the point using a cubic equation
        glm::vec3 getPointOnSpline(float s) const;
    private:
        glm::vec3 a;
        glm::vec3 b;
        glm::vec3 c;
        glm::vec3 d;
    };

    glm::ivec2 threshold;
    std::vector<TransferFunctionColorPoint> colorPoints;
    std::vector<TransferFunctionAlphaPoint> alphaPoints;
    std::vector<Cubic> alphaSpline;
    std::vector<Cubic> colorSpline;
    std::array<glm::vec4, 256> indexedTransferFunction;
    cinder::gl::Texture1dRef transferFunctionTexture;
    static std::vector<Cubic> CalculateCubicSpline(std::vector<glm::vec3> points);
};
