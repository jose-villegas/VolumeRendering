#include "TransferFunctionPoint.h"

TransferFunctionPoint::TransferFunctionPoint(const int isoValue)
{
    this->isoValue = isoValue;
}

const int &TransferFunctionPoint::getIsoValue() const
{
    return isoValue;
}

void TransferFunctionPoint::setIsoValue(const int value)
{
    this->isoValue = value;
}

TransferFunctionPoint::~TransferFunctionPoint() {}

bool TransferFunctionPoint::operator<(const TransferFunctionPoint& rhs) const
{
    return this->isoValue < rhs.getIsoValue();
}

bool TransferFunctionPoint::operator==(const TransferFunctionPoint& rhs) const
{
    return this->isoValue == rhs.getIsoValue();
}

const glm::vec3 &TransferFunctionColorPoint::getColor() const
{
    return color;
}

void TransferFunctionColorPoint::setColor(const glm::vec3& value)
{
    this->color = value;
}

TransferFunctionColorPoint::TransferFunctionColorPoint(const glm::vec3& color, const int iso) : TransferFunctionPoint(iso)
{
    this->color = color;
}

bool TransferFunctionColorPoint::operator<(const TransferFunctionColorPoint& rhs) const
{
    return TransferFunctionPoint::operator<(rhs) ? true : TransferFunctionPoint::operator==(rhs) && length2(this->color) < length2(rhs.color);
}

bool TransferFunctionColorPoint::operator==(const TransferFunctionColorPoint& rhs) const
{
    return TransferFunctionPoint::operator==(rhs) && length2(this->color) == length2(rhs.color);
}

const float &TransferFunctionAlphaPoint::getAlpha() const
{
    return alpha;
}

void TransferFunctionAlphaPoint::setAlpha(const float value)
{
    this->alpha = value;
}

TransferFunctionAlphaPoint::TransferFunctionAlphaPoint(const float alpha, const int iso) : TransferFunctionPoint(iso)
{
    this->alpha = alpha;
}

bool TransferFunctionAlphaPoint::operator<(const TransferFunctionAlphaPoint& rhs) const
{
    return TransferFunctionPoint::operator<(rhs) ? true : TransferFunctionPoint::operator==(rhs) && this->alpha < rhs.alpha;
}

bool TransferFunctionAlphaPoint::operator==(const TransferFunctionAlphaPoint& rhs) const
{
    return TransferFunctionPoint::operator==(rhs) && this->alpha == rhs.alpha;
}
