#pragma once
#include <string>
class RaycastVolume;

class VolumeRenderingAppUi
{
public:
    static bool PostProcessingEnabled();
    static void DrawUi(RaycastVolume &volume);
private:
    static bool VolumeLoadPopup(RaycastVolume &volume);
    static void DrawRenderingOptions(RaycastVolume &volume);
    static void DrawLightingSetup(RaycastVolume &volume);
    static void DrawRotationControls(RaycastVolume &volume);
    static void DrawMainMenuBar(RaycastVolume &volume);

    static bool enablePostProcessing;
    static bool loadNewVolume;
    static bool showVolumeOptions;
    static bool showRendering;
    static bool showTransferFunction;
    static bool showRotationControls;
    static bool showLightingSetup;
    static std::string path;
};

