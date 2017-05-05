#include "VolumeRenderingAppUi.h"
#include "RaycastVolume.h"
#include "CinderImGui.h"
#include "TransferFunctionUi.h"
#include "RenderingParams.h"
using namespace glm;

bool VolumeRenderingAppUi::enablePostProcessing = true;
bool VolumeRenderingAppUi::loadNewVolume = false;
bool VolumeRenderingAppUi::showVolumeOptions = false;
bool VolumeRenderingAppUi::showRendering = false;
bool VolumeRenderingAppUi::showTransferFunction = false;
bool VolumeRenderingAppUi::showRotationControls = false;
bool VolumeRenderingAppUi::showLightingSetup = false;
std::string VolumeRenderingAppUi::path;

bool VolumeRenderingAppUi::PostProcessingEnabled()
{
    return enablePostProcessing;
}

void VolumeRenderingAppUi::DrawUi(RaycastVolume& volume)
{
    // menu bar
    DrawMainMenuBar(volume);
    // rotation controls
    DrawRotationControls(volume);
    // model lighting setup
    DrawLightingSetup(volume);
    // volume controls
    DrawRenderingOptions(volume);

    // open modal dialog for volume parameters
    if (loadNewVolume)
    {
        ui::OpenPopup("Volume Parameters");
        bool isClosed = VolumeLoadPopup(volume);
        loadNewVolume = !isClosed;
        showVolumeOptions = isClosed;
    }

    // initialize transfer function ui
    static std::shared_ptr<TransferFunctionUi> transferFunctionUi = nullptr;

    if(!transferFunctionUi)
    {
        transferFunctionUi = std::make_shared<TransferFunctionUi>();
        volume.setTransferFunction(transferFunctionUi);
    }
    else
    {
        transferFunctionUi->drawUi(showTransferFunction, volume);
    }
}

void VolumeRenderingAppUi::DrawRenderingOptions(RaycastVolume& volume)
{
    if (showRendering)
    {
        ui::Begin("Rendering", &showRendering, ImGuiWindowFlags_AlwaysAutoResize);
        static float stepScale = volume.getStepScale();
        static vec3 aspectRatios = volume.getAspectRatios();
        static int renderingPath = 0;
        static float exposure = RenderingParams::GetExposure();
        static float gamma = RenderingParams::GetGamma();
        stepScale = volume.getStepScale();
        aspectRatios = volume.getAspectRatios();

        if (ui::SliderFloat("Step Scale", &stepScale, 0.1, 8))
        {
            volume.setStepScale(stepScale);
        }

        if (ui::InputFloat3("Aspect", value_ptr(aspectRatios)))
        {
            volume.setAspectratios(aspectRatios);
        }

        if(ui::SliderFloat("Gamma", &gamma, 0.01f, 10.0f))
        {
            RenderingParams::SetGamma(gamma);
        }

        if (ui::SliderFloat("Exposure", &exposure, 0.01f, 8.0f))
        {
            RenderingParams::SetExposure(exposure);
        }

        ui::Checkbox("Enable Post-processing", &enablePostProcessing);

        if(enablePostProcessing)
        {
            static bool fxaa = RenderingParams::FXAAEnabled();

            if (ui::Checkbox("FXAA", &fxaa))
            {
                RenderingParams::FXAAEnabled(fxaa);
            }
        }

        ui::End();
    }
}

void VolumeRenderingAppUi::DrawLightingSetup(RaycastVolume& volume)
{
    if (showLightingSetup)
    {
        static Light light = volume.getLight();
        static float lightIntensity = 1.0f;
        static vec3 rotation;
        static bool doShading = true;
        bool changed = false;
        ui::Begin("Lighting", &showLightingSetup, ImGuiWindowFlags_AlwaysAutoResize);

        if (ui::Checkbox("Enable Diffuse Shading", &doShading))
        {
            volume.diffuseShading(doShading);
        }

        changed |= ui::SliderFloat3("Rotation", value_ptr(rotation), -180, 180);
        changed |= ui::DragFloat3("Ambient", value_ptr(light.ambient), 0.01, 0, 1);
        changed |= ui::DragFloat3("Diffuse", value_ptr(light.diffuse), 0.01, 0, 1);
        changed |= ui::DragFloat("Intensity", &lightIntensity, 0.1, 0, 32);

        if (changed)
        {
            vec3 rads = radians(rotation);
            light.direction = vec3(0, 0, 1) * mat3(eulerAngleXYZ(rads.x, rads.y, rads.z));
            volume.setLight(light.direction, light.ambient, light.diffuse * lightIntensity);
        }

        ui::End();
    }
}

void VolumeRenderingAppUi::DrawRotationControls(RaycastVolume& volume)
{
    if (showRotationControls)
    {
        static vec3 angles = vec3(0);
        bool changed = false;
        ui::Begin("Rotate", &showRotationControls, ImGuiWindowFlags_AlwaysAutoResize);
        changed |= ui::SliderAngle("X", &angles.x);
        changed |= ui::SliderAngle("Y", &angles.y);
        changed |= ui::SliderAngle("Z", &angles.z);

        if (changed)
        {
            volume.setRotation(toQuat(eulerAngleXYZ(angles.x, angles.y, angles.z)));
        }

        ui::End();
    }
}

bool VolumeRenderingAppUi::VolumeLoadPopup(RaycastVolume& volume)
{
    bool isClosed = false;

    if (ui::BeginPopupModal("Volume Parameters", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static ivec3 slices = ivec3(1);
        static vec3 ratios = vec3(1);
        static int bits = 0;
        // volume dimensions
        ui::InputInt3("Slices", value_ptr(slices));
        // volume aspectRatios
        ui::InputFloat3("Aspect", value_ptr(ratios));
        // node bit size
        ui::RadioButton("8 bits", &bits, 0);
        ui::SameLine();
        ui::Dummy(ImVec2(ui::GetContentRegionAvailWidth() / 3, 0));
        ui::SameLine();
        ui::RadioButton("16 bits", &bits, 1);
        // slices has to be positive
        slices = max(slices, ivec3(1));

        if (ui::Button("Load", ImVec2(ui::GetContentRegionAvailWidth(), 0)))
        {
            volume.loadFromFile(slices, ratios, path, bits == 1);
            ui::CloseCurrentPopup();
            isClosed = true;
        }

        ui::EndPopup();
    }

    return isClosed;
}

void VolumeRenderingAppUi::DrawMainMenuBar(RaycastVolume& volume)
{
    // menu bar on top
    {
        ui::ScopedMainMenuBar menuBar;

        if (ui::BeginMenu("File"))
        {
            if (ui::MenuItem("Open"))
            {
                auto fspath = cinder::app::getOpenFilePath(path, { "raw" });

                if (!fspath.empty())
                {
                    path = fspath.string();
                    loadNewVolume = true;
                }
            }
            ui::EndMenu();
        }

        if (ui::BeginMenu("Volume", showVolumeOptions))
        {
            ui::MenuItem("Rotate", nullptr, &showRotationControls);
            ui::MenuItem("Rendering", nullptr, &showRendering);
            ui::MenuItem("Lighting", nullptr, &showLightingSetup);
            ui::MenuItem("Transfer Function", nullptr, &showTransferFunction);

            ui::EndMenu();
        }
    }
}