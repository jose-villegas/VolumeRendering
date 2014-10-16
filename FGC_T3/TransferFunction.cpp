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

    if (!controlPoints.empty() && it != controlPoints.end()) { // Real bad code to force isoValue uniqueness
        if ((it)->isoValue == nControlPoint.isoValue) {
            nControlPoint.isoValue--;

            if (nControlPoint.isoValue < 1) { nControlPoint.isoValue = 1; it++;}
        }

        auto uniqItLeft = it;
        auto uniqItRight = it;
        int previusVal = nControlPoint.isoValue;

        while (uniqItLeft != controlPoints.begin()) {
            if ((uniqItLeft - 1)->isoValue == previusVal) {
                (uniqItLeft - 1)->isoValue--;
            }

            previusVal = (uniqItLeft - 1)->isoValue;
            uniqItLeft--;
        }

        while (uniqItRight != controlPoints.end() - 1) {
            if ((uniqItRight + 1)->isoValue == previusVal) {
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
    // TODO
}

void TransferFunction::getLinearFunction(byte dst[256][4])
{
    std::thread channelThread[4];
    std::vector<double> channel[5];
    tk::Spline channelSpline[4];

    // Control Points
    for (int i = 0; i < controlPoints.size(); i++) {
        channel[0].push_back(controlPoints[i].rgba[0] * 255);
        channel[1].push_back(controlPoints[i].rgba[1] * 255);
        channel[2].push_back(controlPoints[i].rgba[2] * 255);
        channel[3].push_back(controlPoints[i].rgba[3] * 255);
        channel[4].push_back(controlPoints[i].isoValue);
    }

    for (int i = 0; i < 4; i++) {
        channelThread[i] = std::thread(channelSplineThread, channelSpline[i], channel[4], &channel[i], &dst[0][i]);
    }

    for (int i = 0; i < 4; i++) {
        channelThread[i].join();
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

void TransferFunction::channelSplineThread(tk::Spline &spline, std::vector<double> isoV, std::vector<double> * channel, byte * dst)
{
    spline.set_points(isoV, *channel, false);
    int min;
    int max;
    int current = 0;
    min = std::numeric_limits<int>::infinity();
    max = 0;
    channel->clear();

    for (int i = 0; i < 256; i++) {
        current = (int)spline(i);
        // Max Value
        max = current > max ? current : max;
        // Min Value
        min = current < min ? current : min;
        channel->push_back(current);
    }

    for (int i = 0; i < 256; i++) {
        *dst = (int)channel->at(i);
        dst += 4 * sizeof(byte);
    }
}


bool operator<(ControlPoint const &a, ControlPoint const &b)
{
    return a.isoValue < b.isoValue;
}

