#include "TransferFunction.h"

std::vector<ControlPoint> TransferFunction::colorControlPoints;
std::vector<ControlPoint> TransferFunction::alphaControlPoints;

void ControlPoint::create(int r, int g, int b, int isovalue)
{
    this->rgba[0] = r;
    this->rgba[1] = g;
    this->rgba[2] = b;
    this->rgba[3] = 1.0f;
    this->isoValue = isovalue;
}

void ControlPoint::create(int alpha, int isovalue)
{
    rgba[0] = 0;
    rgba[1] = 0;
    rgba[2] = 0;
    rgba[3] = alpha;
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
}


bool operator<(ControlPoint const &a, ControlPoint const &b)
{
    return a.isoValue < b.isoValue;
}

bool operator>(ControlPoint const &a, ControlPoint const &b)
{
    return a.isoValue > b.isoValue;
}
