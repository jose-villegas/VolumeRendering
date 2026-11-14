#pragma once

/**
 * \brief Lighting that traces the volume itself: gradient shading with specular,
 * shadows from a light transmittance volume, ambient occlusion and one bounce of
 * indirect light from an ambient volume
 */
struct VolumetricLighting
{
    bool enabled = true;
    float gradientShading = 1.0f;
    // scales the light's ambient color, sky light that ambient occlusion attenuates
    float ambientStrength = 3.0f;
    float specularStrength = 0.35f;
    float shininess = 24.0f;
    bool shadows = true;
    bool occlusion = true;
    // occlusion radius in voxels
    float occlusionRadius = 12.0f;
    bool indirect = true;
    float indirectStrength = 1.0f;
    // lighting volumes resolution divisor, 1 full, 2 half, 4 quarter
    int volumeDivisor = 2;
};

class RenderingParams
{
public:
    static VolumetricLighting& Volumetric();
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

