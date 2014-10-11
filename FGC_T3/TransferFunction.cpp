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
}

void TransferFunction::getLinearFunction(byte dst[256][4])
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

    redSpline.set_points(isoV, red, false);
    greenSpline.set_points(isoV, green, false);
    blueSpline.set_points(isoV, blue, false);
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

    //////////////////////////////////////////////////////////////////////////
    // Alpha Points
    for (int i = 0; i < alphaControlPoints.size(); i++)
    {
        alpha.push_back(alphaControlPoints[i].rgba.a);
        isoV.push_back(alphaControlPoints[i].isoValue);
    }

    alphaSpline.set_points(isoV, alpha, false);
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

    //////////////////////////////////////////////////////////////////////////
    for (int i = 0; i < 256; i++)
    {
        dst[i][0] = (int)(red[i]);
        dst[i][1] = (int)(green[i]);
        dst[i][2] = (int)(blue[i]);
        dst[i][3] = (int)(alpha[i]);
    }
}


bool operator<(ControlPoint const &a, ControlPoint const &b)
{
    return a.isoValue < b.isoValue;
}

bool operator>(ControlPoint const &a, ControlPoint const &b)
{
    return a.isoValue > b.isoValue;
}
