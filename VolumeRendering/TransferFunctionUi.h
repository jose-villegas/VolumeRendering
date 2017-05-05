#pragma once
#include "TransferFunction.h"
class RaycastVolume;

class TransferFunctionUi : public TransferFunction
{
public:
    void drawUi(bool& open, const RaycastVolume& volume);
    TransferFunctionUi();
    ~TransferFunctionUi();
private:
    void drawHistogram(const RaycastVolume& volume) const;
    void drawThresholdControl();
    void drawControlPointsUi();
    void drawControlPointList(int pointType);
    void drawControlPointCreationUi();
};
