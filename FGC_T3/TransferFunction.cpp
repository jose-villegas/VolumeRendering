#include "TransferFunction.h"

std::vector<ControlPoint> TransferFunction::controlPoints;

void ControlPoint::create(int r, int g, int b, int alpha, int isovalue)
{
    this->rgba[0] = (float)r / 255.0;
    this->rgba[1] = (float)g / 255.0;
    this->rgba[2] = (float)b / 255.0;
    this->rgba[3] = (float)alpha / 255.0;
    this->isoValue = isovalue;
}

void TransferFunction::addControlPoint(int r, int g, int b, int alpha, int isovalue)
{
    if (alpha < 0) { alpha = 0; }

    if (isovalue < 0) { isovalue = 0; }

    if (alpha > 255) { alpha = 255; }

    if (isovalue > 255) { isovalue = 255; }

    ControlPoint nControlPoint;
    nControlPoint.create(r, g, b, alpha, isovalue);
    auto it = std::lower_bound(controlPoints.begin(), controlPoints.end(), nControlPoint);

    if (!controlPoints.empty() && it != controlPoints.end()) // Real bad code to force isoValue uniqueness
    {
        if ((it)->isoValue == nControlPoint.isoValue)
        {
            nControlPoint.isoValue--;

            if (nControlPoint.isoValue < 1) { nControlPoint.isoValue = 1; it++;}
        }

        auto uniqItLeft = it;
        auto uniqItRight = it;
        int previusVal = nControlPoint.isoValue;

        while (uniqItLeft != controlPoints.begin())
        {
            if ((uniqItLeft - 1)->isoValue == previusVal)
            {
                (uniqItLeft - 1)->isoValue--;
            }

            previusVal = (uniqItLeft - 1)->isoValue;
            uniqItLeft--;
        }

        while (uniqItRight != controlPoints.end() - 1)
        {
            if ((uniqItRight + 1)->isoValue == previusVal)
            {
                (uniqItRight + 1)->isoValue++;
            }

            previusVal = (uniqItRight - 1)->isoValue;
            uniqItRight++;
        }
    }

    controlPoints.insert(it, nControlPoint);
}

void TransferFunction::getSmoothFunction(byte dst[256][4])
{
}

void TransferFunction::getLinearFunction(byte dst[256][4])
{
    tk::Spline alphaSpline, redSpline, greenSpline, blueSpline;
    std::vector<double> alpha, red, green, blue, isoV;

    //////////////////////////////////////////////////////////////////////////
    // Control Points
    for (int i = 0; i < controlPoints.size(); i++)
    {
        red.push_back(controlPoints[i].rgba[0] * 255);
        green.push_back(controlPoints[i].rgba[1] * 255);
        blue.push_back(controlPoints[i].rgba[2] * 255);
        alpha.push_back(controlPoints[i].rgba[3] * 255);
        isoV.push_back(controlPoints[i].isoValue);
    }

    redSpline.set_points(isoV, red, false);
    greenSpline.set_points(isoV, green, false);
    blueSpline.set_points(isoV, blue, false);
    alphaSpline.set_points(isoV, alpha, false);
    int minRed, minGreen, minBlue, minAlpha;
    int maxRed, maxGreen, maxBlue, maxAlpha;
    int r = 0, g = 0, b = 0, a = 0;
    minRed = minGreen = minBlue = minAlpha = std::numeric_limits<int>::infinity();
    maxRed = maxGreen = maxBlue = maxAlpha = 0;
    red.clear(); green.clear(); blue.clear(); isoV.clear(), alpha.clear();

    for (int i = 0; i < 256; i++)
    {
        r = (int)redSpline(i);
        g = (int)greenSpline(i);
        b = (int)blueSpline(i);
        a = (int)alphaSpline(i);
        // Max Value
        maxRed = r > maxRed ? r : maxRed;
        maxGreen = g > maxGreen ? g : maxGreen;
        maxBlue = b > maxBlue ? b : maxBlue;
        maxAlpha = a > maxAlpha ? a : maxAlpha;
        // Min Value
        minRed = r < minRed ? r : minRed;
        minGreen = g < minGreen ? g : minGreen;
        minBlue = b < minBlue ? b : minBlue;
        minAlpha = a < minAlpha ? a : minAlpha;
        red.push_back(r);
        green.push_back(g);
        blue.push_back(b);
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

void TransferFunction::deleteAlphaControlPoint(unsigned const int index)
{
    controlPoints.erase(controlPoints.begin() + index);
}

float * TransferFunction::getControlPointColors(unsigned const int index)
{
    return controlPoints[index].rgba;
}


bool operator<(ControlPoint const &a, ControlPoint const &b)
{
    return a.isoValue < b.isoValue;
}

