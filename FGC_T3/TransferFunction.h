#pragma
#include "commons.h"
#include "Spline.h"

class ControlPoint
{
    public:
        glm::vec4 rgba;
        int isoValue;

        void create(int r, int g, int b, int isovalue);
        void create(int alpha, int isovalue);

        friend bool operator<(ControlPoint const &a, ControlPoint const &b);
};

class TransferFunction
{
    private:
        static std::vector<ControlPoint> colorControlPoints;
        static std::vector<ControlPoint> alphaControlPoints;
    public:
        static void addControlPoint(int r, int g, int b, int isovalue);
        static void addControlPoint(int alpha, int isovalue);
        static void deleteAlphaControlPoint(unsigned const int index);
        static void getSmoothFunction(byte dst[256][4]);
        static void getLinearFunction(byte dst[256][4]);
        static std::vector<ControlPoint> getAlphaControlPoints() { return alphaControlPoints; }
        static std::vector<ControlPoint> getColorControlPoints() { return colorControlPoints; }
};