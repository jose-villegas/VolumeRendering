#pragma
#include "commons.h"
#include "Spline.h"

class ControlPoint
{
    public:
        float rgba[4];
        int isoValue;

        void create(int r, int g, int b, int alpha, int isovalue);

        friend bool operator<(ControlPoint const &a, ControlPoint const &b);
};

class TransferFunction
{
    private:
        static std::vector<ControlPoint> controlPoints;
    public:
        static void addControlPoint(int r, int g, int b, int alpha, int isovalue);
        static void deleteAlphaControlPoint(unsigned const int index);
        static void getSmoothFunction(byte dst[256][4]);
        static void getLinearFunction(byte dst[256][4]);
        static void channelSplineThread(tk::Spline &spline, std::vector<double> isoV, std::vector<double> * channel, byte * dst);
        static float * getControlPointColors(unsigned const int index);
        static std::vector<ControlPoint> getControlPoints() { return controlPoints; }

};