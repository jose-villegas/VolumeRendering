#pragma once
#include "TransferFunctionPoint.h"
#include "CubicSpline.h"
class RaycastVolume;

class TransferFunction
{
public:
    TransferFunction();
    virtual ~TransferFunction();
    glm::vec4 getColor(const float t /* t=[0..1] */);
    glm::vec4 getColor(const int isoValue /* isoValue=[0..255] */);
    virtual void updateFunction();
    void addColorPoint(const glm::vec3& color, const int isoValue);
    void addAlphaPoint(const float alpha, const int isoValue);
    void removeColorPoint(const int index);
    void removeAlphaPoint(const int index);
    void setAlpha(const int index, const float alpha);
    void setColor(const int index, const glm::vec3 &color);
    void setAlphaPointIsoValue(const int index, const int isoValue);
    void setColorPointIsoValue(const int index, const int isoValue);
    void setThreshold(int minIso, int maxIso);
    const glm::ivec2 &getThreshold() const;
    const std::vector<TransferFunctionColorPoint> &getColorPoints() const;
    const std::vector<TransferFunctionAlphaPoint> &getAlphaPoints() const;
    const std::array<glm::vec4, 256> &getIndexedTransferFunction() const;
    const cinder::gl::Texture1dRef &getColorMappingTexture();
private:
    glm::ivec2 threshold;
    std::vector<TransferFunctionColorPoint> colorPoints;
    std::vector<TransferFunctionAlphaPoint> alphaPoints;
    std::vector<CubicSpline> alphaSpline;
    std::vector<CubicSpline> colorSpline;
    std::array<glm::vec4, 256> indexedTransferFunction;
    bool updateColorTexture;

    cinder::gl::Texture1dRef colorMappingTexture;
};
