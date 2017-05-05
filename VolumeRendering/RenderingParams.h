#pragma once
class RenderingParams
{
public:
    static float GetExposure();
    static void SetExposure(const float exposure);
    static float GetGamma();
    static void SetGamma(const float gamma);
    static void FXAAEnabled(const bool enabled);
    static bool FXAAEnabled();
private:
    static float gammaValue;
    static float exposureValue;
    static bool fxaa;
};

