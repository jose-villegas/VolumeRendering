#pragma once
#include "cinder/gl/gl.h"

class TransferFunctionPoint
{
protected:
    TransferFunctionPoint() = default;
    explicit TransferFunctionPoint(const int isoValue);
    int isoValue;
public:
    const int &getIsoValue() const;
    void setIsoValue(const int value);
    ~TransferFunctionPoint();

    bool operator<(const TransferFunctionPoint& rhs) const;
    bool operator==(const TransferFunctionPoint& rhs) const;
};

class TransferFunctionColorPoint : public TransferFunctionPoint
{
public:
    const glm::vec3 &getColor() const;
    void setColor(const glm::vec3& value);
    TransferFunctionColorPoint() = default;
    TransferFunctionColorPoint(const glm::vec3& color, const int iso);
    bool operator<(const TransferFunctionColorPoint& rhs) const;
    bool operator==(const TransferFunctionColorPoint& rhs) const;
private:
    glm::vec3 color;
};

class TransferFunctionAlphaPoint : public TransferFunctionPoint
{
public:
    const float &getAlpha() const;
    void setAlpha(const float value);
    TransferFunctionAlphaPoint() = default;
    TransferFunctionAlphaPoint(const float alpha, const int iso);
    bool operator<(const TransferFunctionAlphaPoint& rhs) const;
    bool operator==(const TransferFunctionAlphaPoint& rhs) const;;
private:
    float alpha;
};
