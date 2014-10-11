#include "TransferFunction.h"

std::vector<ControlPoint> TransferFunction::colorControlPoints;
std::vector<ControlPoint> TransferFunction::alphaControlPoints;

void ControlPoint::create(int r, int g, int b, int isovalue)
{
    this->rgba.r = r;
    this->rgba.g = g;
    this->rgba.b = b;
    this->rgba.a = 1.0f;
    this->isoValue = isovalue;
}

void ControlPoint::create(int alpha, int isovalue)
{
    this->rgba.r = 0;
    this->rgba.g = 0;
    this->rgba.b = 0;
    this->rgba.a = alpha;
    this->isoValue = isovalue;
}


void TransferFunction::addControlPoint(int r, int g, int b, int isovalue)
{
    ControlPoint nControlPoint;
    nControlPoint.create(r, g, b, isovalue);
    auto it = std::lower_bound(colorControlPoints.begin(), colorControlPoints.end(), nControlPoint);
    colorControlPoints.insert(it, nControlPoint);
}

void TransferFunction::addControlPoint(int alpha, int isovalue)
{
    ControlPoint nControlPoint;
    nControlPoint.create(alpha, isovalue);
    auto it = std::lower_bound(alphaControlPoints.begin(), alphaControlPoints.end(), nControlPoint);
    alphaControlPoints.insert(it, nControlPoint);
}

void TransferFunction::getSmoothFunction(byte dst[256][4])
{
    tk::Spline alphaSpline, redSpline, greenSpline, blueSpline;
    std::vector<double> alpha, red, green, blue, isoV;

    //////////////////////////////////////////////////////////////////////////
    // Color Points
    for (int i = 0; i < colorControlPoints.size(); i++)
    {
        red.push_back(colorControlPoints[i].rgba.r);
        green.push_back(colorControlPoints[i].rgba.g);
        blue.push_back(colorControlPoints[i].rgba.b);
        isoV.push_back(colorControlPoints[i].isoValue);
    }

    redSpline.set_points(isoV, red, true);
    greenSpline.set_points(isoV, green, true);
    blueSpline.set_points(isoV, blue, true);
    int minRed, minGreen, minBlue;
    int maxRed, maxGreen, maxBlue;
    int r = 0, g = 0, b = 0;
    minRed = minGreen = minBlue = std::numeric_limits<int>::infinity();
    maxRed = maxGreen = maxBlue = 0;
    red.clear(); green.clear(); blue.clear(); isoV.clear();

    for (int i = 0; i < 256; i++)
    {
        r = (int)redSpline(i);
        g = (int)greenSpline(i);
        b = (int)blueSpline(i);
        // Max Value
        maxRed = r > maxRed ? r : maxRed;
        maxGreen = g > maxGreen ? g : maxGreen;
        maxBlue = b > maxBlue ? b : maxBlue;
        // Min Value
        minRed = r < minRed ? r : minRed;
        minGreen = g < minGreen ? g : minGreen;
        minBlue = b < minBlue ? b : minBlue;
        red.push_back(r);
        green.push_back(g);
        blue.push_back(b);
    }

    std::transform(red.begin(), red.end(), red.begin(), std::bind1st(std::plus<int>(), -minRed));
    std::transform(green.begin(), green.end(), green.begin(), std::bind1st(std::plus<int>(), -minGreen));
    std::transform(blue.begin(), blue.end(), blue.begin(), std::bind1st(std::plus<int>(), -minBlue));
    maxRed += -minRed; maxGreen += -minGreen; maxBlue += -minBlue;
    std::transform(red.begin(), red.end(), red.begin(), std::bind1st(std::multiplies<double>(), 1.0f / maxRed));
    std::transform(green.begin(), green.end(), green.begin(), std::bind1st(std::multiplies<double>(),  1.0f / maxGreen));
    std::transform(blue.begin(), blue.end(), blue.begin(), std::bind1st(std::multiplies<double>(), 1.0f / maxBlue));

    //////////////////////////////////////////////////////////////////////////
    // Alpha Points
    for (int i = 0; i < alphaControlPoints.size(); i++)
    {
        alpha.push_back(alphaControlPoints[i].rgba.a);
        isoV.push_back(alphaControlPoints[i].isoValue);
    }

    alphaSpline.set_points(isoV, alpha, true);
    int a, maxAlpha = 0, minAlpha = std::numeric_limits<int>::infinity();
    alpha.clear(); isoV.clear();

    for (int i = 0; i < 256; i++)
    {
        a = (int)alphaSpline(i);
        // Max Value
        maxAlpha = a > maxAlpha ? a : maxAlpha;
        // Min Value
        minAlpha = a < minAlpha ? a : minAlpha;
        alpha.push_back(a);
    }

    std::transform(alpha.begin(), alpha.end(), alpha.begin(), std::bind1st(std::plus<int>(), -minAlpha));
    maxAlpha += -minAlpha;
    std::transform(alpha.begin(), alpha.end(), alpha.begin(), std::bind1st(std::multiplies<double>(), 1.0f / maxAlpha));

    //////////////////////////////////////////////////////////////////////////
    for (int i = 0; i < 256; i++)
    {
        dst[i][0] = (int)(255.0 * red[i]);
        dst[i][1] = (int)(255.0 * green[i]);
        dst[i][2] = (int)(255.0 * blue[i]);
        dst[i][3] = (int)(255.0 * alpha[i]);
    }
}

void TransferFunction::getLinearFunction(byte dst[256][4])
{
}


bool operator<(ControlPoint const &a, ControlPoint const &b)
{
    return a.isoValue < b.isoValue;
}

bool operator>(ControlPoint const &a, ControlPoint const &b)
{
    return a.isoValue > b.isoValue;
}
