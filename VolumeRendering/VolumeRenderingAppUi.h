#pragma once
#include <memory>
#include <string>
#include <cinder/CinderGlm.h>

class RaycastVolume;
class TransferFunctionUi;

class VolumeRenderingAppUi
{
public:
    /**
     * \brief Creates the transfer function editor and applies the default light to the volume
     */
    static void Initialize(RaycastVolume &volume);
    static bool PostProcessingEnabled();
    static void PostProcessingEnabled(bool enabled) { enablePostProcessing = enabled; }
    static void DrawUi(RaycastVolume &volume);
    /**
     * \brief Sets the light rotation, in degrees, and intensity and applies them to the volume
     */
    static void SetLight(RaycastVolume &volume, const glm::vec3 &rotation, float intensity);
    static const glm::vec3 &GetLightRotation() { return lightRotation; }
    /**
     * \brief Opens the tool windows, used for screenshots
     */
    static void ShowWindows(bool rendering, bool lighting, bool transferFunction);
private:
    static bool VolumeLoadPopup(RaycastVolume &volume);
    static void DrawRenderingOptions(RaycastVolume &volume);
    static void DrawLightingSetup(RaycastVolume &volume);
    static void DrawRotationControls(RaycastVolume &volume);
    static void DrawMainMenuBar(RaycastVolume &volume);
    static void ApplyLight(RaycastVolume &volume);

    static bool enablePostProcessing;
    static bool loadNewVolume;
    static bool showVolumeOptions;
    static bool showRendering;
    static bool showTransferFunction;
    static bool showRotationControls;
    static bool showLightingSetup;
    static std::string path;
    static std::shared_ptr<TransferFunctionUi> transferFunctionUi;

    // light setup, rotation in degrees
    static glm::vec3 lightRotation;
    static glm::vec3 lightAmbient;
    static glm::vec3 lightDiffuse;
    static float lightIntensity;
    static bool diffuseShading;
};
