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

    updateFunction();
}

TransferFunction::~TransferFunction() {}

std::vector<TransferFunction::Cubic> TransferFunction::CalculateCubicSpline(std::vector<vec3> points)
{
    auto n = points.size() - 1;
    auto& v = points;
    std::vector<vec3> gamma(n + 1);
    std::vector<vec3> delta(n + 1);
    std::vector<vec3> D(n + 1);

    int i;
    /* We need to solve the equation
    * taken from: http://mathworld.wolfram.com/CubicSpline.html
    [2 1       ] [D[0]]   [3(v[1] - v[0])  ]
    |1 4 1     | |D[1]|   |3(v[2] - v[0])  |
    |  1 4 1   | | .  | = |      .         |
    |    ..... | | .  |   |      .         |
    |     1 4 1| | .  |   |3(v[n] - v[n-2])|
    [       1 2] [D[n]]   [3(v[n] - v[n-1])]

    by converting the matrix to upper triangular.
    The D[i] are the derivatives at the control points.
    */

    //this builds the coefficients of the left matrix
    gamma[0] = vec3(0);
    gamma[0].x = 1.0f / 2.0f;
    gamma[0].y = 1.0f / 2.0f;
    gamma[0].z = 1.0f / 2.0f;

    for (i = 1; i < n; i++)
    {
        gamma[i] = vec3(1) / ((4.0f * vec3(1)) - gamma[i - 1]);
    }

    gamma[n] = vec3(1) / ((2.0f * vec3(1)) - gamma[n - 1]);

    delta[0] = 3.0f * (v[1] - v[0]) * gamma[0];

    for (i = 1; i < n; i++)
    {
        delta[i] = (3.0f * (v[i + 1] - v[i - 1]) - delta[i - 1]) * gamma[i];
    }

    delta[n] = (3.0f * (v[n] - v[n - 1]) - delta[n - 1]) * gamma[n];

    D[n] = delta[n];

    for (i = n - 1; i >= 0; i--)
    {
        D[i] = delta[i] - gamma[i] * D[i + 1];
    }

    // now compute the coefficients of the cubics 
    std::vector<Cubic> C(n);

    for (i = 0; i < n; i++)
    {
        C[i] = Cubic(v[i], D[i], 3.0f * (v[i + 1] - v[i]) - 2.0f * D[i] - D[i + 1], 2.0f * (v[i] - v[i + 1]) + D[i] + D[i + 1]);
    }

    return C;
}

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

void TransferFunction::removeColorPoint(const int isoValue)
{
    // off limits
    if (isoValue <= 0 || isoValue >= colorPoints.back().getIsoValue())
    {
        return;
    }

    colorPoints.erase(std::remove_if(colorPoints.begin(), colorPoints.end(),
                                     [=](const TransferFunctionColorPoint& p)
                                     {
                                         return p.getIsoValue() == isoValue;
                                     }));
    updateFunction();
}

void TransferFunction::removeAlphaPoint(const int isoValue)
{
    // off limits
    if (isoValue <= 0 || isoValue >= alphaPoints.back().getIsoValue())
    {
        return;
    }

    alphaPoints.erase(std::remove_if(alphaPoints.begin(), alphaPoints.end(),
                                     [=](const TransferFunctionAlphaPoint& p)
                                     {
                                         return p.getIsoValue() == isoValue;
                                     }));
    updateFunction();
}

void TransferFunction::setThreshold(int minIso, int maxIso)
{
    minIso = min(max(0, minIso), maxIso - 1);
    maxIso = max(min(255, maxIso), minIso - 1);

    threshold.x = minIso;
    threshold.y = maxIso;
}

const ivec2& TransferFunction::getThreshold() const
{
    return threshold;
}

TransferFunction::Cubic::Cubic(vec3 a, vec3 b, vec3 c, vec3 d)
{
    this->a = a;
    this->b = b;
    this->c = c;
    this->d = d;
}

vec3 TransferFunction::Cubic::getPointOnSpline(float s) const
{
    return (((d * s) + c) * s + b) * s + a;
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

    alphaSpline = move(CalculateCubicSpline(alphaCtrPoints));
    colorSpline = move(CalculateCubicSpline(colorCtrPoints));

    // update fast access transfer function
    for (int i = 0; i < indexedTransferFunction.size(); i++)
    {
        indexedTransferFunction[i] = getColor(static_cast<float>(i) / 255);
    }

    // update texture
    if (!transferFunctionTexture)
    {
        auto format = gl::Texture1d::Format().minFilter(GL_LINEAR)
                                             .magFilter(GL_LINEAR)
                                             .wrapS(GL_CLAMP_TO_EDGE)
                                             .internalFormat(GL_RGBA);
        format.setDataType(GL_FLOAT);
        transferFunctionTexture = gl::Texture1d::create(indexedTransferFunction.data(), GL_RGBA, 256, format);
    }
    else
    {
        transferFunctionTexture->update(indexedTransferFunction.data(), GL_RGBA, GL_FLOAT, 0, 256, 0);
    }
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

const gl::Texture1dRef &TransferFunction::get1DTexture() const
{
    return transferFunctionTexture;
}
