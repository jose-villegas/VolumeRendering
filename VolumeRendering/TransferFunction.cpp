#include "TransferFunction.h"
#include "RaycastVolume.h"
#include "CinderImGui.h"
using namespace glm;
using namespace cinder;

TransferFunction::TransferFunction() : threshold(vec2(0, 255))
{
    colorPoints.push_back(TransferFunctionColorPoint(vec3(1), 0));
    colorPoints.push_back(TransferFunctionColorPoint(vec3(1), 255));

    alphaPoints.push_back(TransferFunctionAlphaPoint(1.0, 0));
    alphaPoints.push_back(TransferFunctionAlphaPoint(1.0, 255));
}

TransferFunction::~TransferFunction() {}

void TransferFunction::addColorPoint(const vec3& color, const int isoValue)
{
    // inserted point is off limits
    if (isoValue <= 0 || isoValue >= colorPoints.back().getIsoValue() || colorPoints.size() > 255)
    {
        return;
    }

    auto ctrlP = TransferFunctionColorPoint(color, isoValue);
    // sorted insert
    colorPoints.insert(upper_bound(colorPoints.begin(), colorPoints.end(), ctrlP), ctrlP);
    updateFunction();
}

void TransferFunction::addAlphaPoint(const float alpha, const int isoValue)
{
    // inserted point is off limits
    if (isoValue <= 0 || isoValue >= alphaPoints.back().getIsoValue() || colorPoints.size() > 255)
    {
        return;
    }

    auto ctrlP = TransferFunctionAlphaPoint(alpha, isoValue);
    // sorted insert
    alphaPoints.insert(upper_bound(alphaPoints.begin(), alphaPoints.end(), ctrlP), ctrlP);
    updateFunction();
}

void TransferFunction::removeColorPoint(const int index)
{
    // off limits
    if (index <= 0 || index >= colorPoints.size())
    {
        return;
    }

    colorPoints.erase(colorPoints.begin() + index);
    updateFunction();
}

void TransferFunction::removeAlphaPoint(const int index)
{
    // off limits
    if (index <= 0 || index >= alphaPoints.size())
    {
        return;
    }

    alphaPoints.erase(alphaPoints.begin() + index);
    updateFunction();
}

void TransferFunction::setAlpha(const int index, const float alpha)
{
    // off limits
    if (index < 0 || index >= alphaPoints.size())
    {
        return;
    }

    alphaPoints[index].setAlpha(clamp(alpha, 0.0f, 1.0f));
    updateFunction();
}

void TransferFunction::setThreshold(int minIso, int maxIso)
{
    minIso = clamp(minIso, 0, maxIso - 1);
    maxIso = clamp(maxIso, minIso + 1, 255);

    threshold.x = minIso;
    threshold.y = maxIso;
}

const ivec2& TransferFunction::getThreshold() const
{
    return threshold;
}

vec4 TransferFunction::getColor(const float t)
{
    float alpha = 0;
    vec3 color = vec3(0);
    int i = 0;
    int isoValue = t * 255;

    for (auto it = alphaPoints.cbegin(); it != --alphaPoints.cend() && i < alphaSpline.size();)
    {
        int currentIso = it->getIsoValue();
        int nextIso = (++it)->getIsoValue();

        // find cubic index
        if (isoValue >= currentIso && isoValue <= nextIso)
        {
            auto& currentCubic = alphaSpline[i];
            float evalAt = static_cast<float>(isoValue - currentIso) / (nextIso - currentIso);
            alpha = currentCubic.getPointOnSpline(evalAt).r;
            break;
        }

        i++;
    }

    i = 0;

    for (auto it = colorPoints.cbegin(); it != --colorPoints.cend() && i < colorSpline.size();)
    {
        int currentIso = it->getIsoValue();
        int nextIso = (++it)->getIsoValue();

        // find cubic index
        if (isoValue >= currentIso && isoValue <= nextIso)
        {
            auto& currentCubic = colorSpline[i];
            float evalAt = static_cast<float>(isoValue - currentIso) / (nextIso - currentIso);
            color = currentCubic.getPointOnSpline(evalAt);
            break;
        }

        i++;
    }

    return vec4(color.r, color.g, color.b, alpha);
}

vec4 TransferFunction::getColor(const int isoValue)
{
    if (isoValue < 0 || isoValue > 255) return vec4(-1);

    return indexedTransferFunction[isoValue];
}

void TransferFunction::updateFunction()
{
    std::vector<vec3> alphaCtrPoints(alphaPoints.size());
    std::vector<vec3> colorCtrPoints(colorPoints.size());

    for (int i = 0; i < alphaCtrPoints.size(); i++)
    {
        alphaCtrPoints[i] = vec3(alphaPoints[i].getAlpha());
    }

    for (int i = 0; i < colorCtrPoints.size(); i++)
    {
        colorCtrPoints[i] = colorPoints[i].getColor();
    }

    alphaSpline = move(CubicSpline::CalculateCubicSpline(alphaCtrPoints));
    colorSpline = move(CubicSpline::CalculateCubicSpline(colorCtrPoints));

    // update fast access transfer function
    for (int i = 0; i < indexedTransferFunction.size(); i++)
    {
        indexedTransferFunction[i] = getColor(static_cast<float>(i) / 255);
    }

    // update texture needs to be update on next query
    updateColorTexture = true;
}

const std::vector<TransferFunctionColorPoint> &TransferFunction::getColorPoints() const
{
    return colorPoints;
}

const std::vector<TransferFunctionAlphaPoint> &TransferFunction::getAlphaPoints() const
{
    return alphaPoints;
}

const std::array<vec4, 256> &TransferFunction::getIndexedTransferFunction() const
{
    return indexedTransferFunction;
}

const gl::Texture1dRef& TransferFunction::getColorMappingTexture()
{
    // update texture
    if (!colorMappingTexture)
    {
        updateFunction();
        auto format = gl::Texture1d::Format().minFilter(GL_LINEAR)
            .magFilter(GL_LINEAR)
            .wrapS(GL_CLAMP_TO_EDGE)
            .internalFormat(GL_RGBA);
        format.setDataType(GL_FLOAT);
        colorMappingTexture = gl::Texture1d::create(getIndexedTransferFunction().data(), GL_RGBA, 256, format);
    }
    else if (updateColorTexture)
    {
        colorMappingTexture->update(getIndexedTransferFunction().data(), GL_RGBA, GL_FLOAT, 0, 256, 0);
        updateColorTexture = true;
    }

    return colorMappingTexture;
}

void TransferFunction::setColor(const int index, const vec3& color)
{
    // off limits
    if (index < 0 || index >= colorPoints.size())
    {
        return;
    }

    colorPoints[index].setColor(clamp(color, vec3(0), vec3(1)));
    updateFunction();
}

void TransferFunction::setAlphaPointIsoValue(const int index, const int isoValue)
{
    // off limits
    if (index <= 0 || index >= alphaPoints.size())
    {
        return;
    }

    // off limits iso val
    if (isoValue <= 0 || isoValue >= 255)
    {
        return;
    }

    alphaPoints[index].setIsoValue(isoValue);

    // function points may need sorting after modiying sorting token, isoValue
    bool needSorting = !is_sorted(alphaPoints.begin(), alphaPoints.end());

    if (needSorting) { sort(alphaPoints.begin(), alphaPoints.end()); }

    updateFunction();
}

void TransferFunction::setColorPointIsoValue(const int index, const int isoValue)
{
    // off limits
    if (index <= 0 || index >= colorPoints.size())
    {
        return;
    }

    // off limits iso val
    if (isoValue <= 0 || isoValue >= 255)
    {
        return;
    }

    colorPoints[index].setIsoValue(isoValue);

    // function points may need sorting after modiying sorting token, isoValue
    bool needSorting = !is_sorted(colorPoints.begin(), colorPoints.end());

    if (needSorting) { sort(colorPoints.begin(), colorPoints.end()); }

    updateFunction();
}
