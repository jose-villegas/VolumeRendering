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
std::shared_ptr<TransferFunctionUi> VolumeRenderingAppUi::transferFunctionUi;
// light from the upper left, in front of the initial camera
vec3 VolumeRenderingAppUi::lightRotation = vec3(-42.0f, 40.0f, 0.0f);
vec3 VolumeRenderingAppUi::lightAmbient = vec3(0.1f);
vec3 VolumeRenderingAppUi::lightDiffuse = vec3(1.0f);
float VolumeRenderingAppUi::lightIntensity = 1.0f;
bool VolumeRenderingAppUi::diffuseShading = true;

void VolumeRenderingAppUi::Initialize(RaycastVolume& volume)
{
    transferFunctionUi = std::make_shared<TransferFunctionUi>();
    volume.setTransferFunction(transferFunctionUi);
    ApplyLight(volume);
}

void VolumeRenderingAppUi::ApplyLight(RaycastVolume& volume)
{
    vec3 rads = radians(lightRotation);
    vec3 direction = vec3(0, 0, 1) * mat3(eulerAngleXYZ(rads.x, rads.y, rads.z));
    volume.setLight(direction, lightAmbient, lightDiffuse * lightIntensity);
}

void VolumeRenderingAppUi::SetLight(RaycastVolume& volume, const vec3& rotation, float intensity)
{
    lightRotation = rotation;
    lightIntensity = intensity;
    ApplyLight(volume);
}

void VolumeRenderingAppUi::ShowWindows(bool rendering, bool lighting, bool transferFunction)
{
    showVolumeOptions = true;
    showRendering = rendering;
    showLightingSetup = lighting;
    showTransferFunction = transferFunction;
}

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

    transferFunctionUi->drawUi(showTransferFunction, volume);
}

void VolumeRenderingAppUi::DrawRenderingOptions(RaycastVolume& volume)
{
    if (showRendering)
    {
        ui::SetNextWindowPos(ImVec2(20, ci::app::getWindowHeight() - 250.0f), ImGuiSetCond_FirstUseEver);
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
        ui::SetNextWindowPos(ImVec2(ci::app::getWindowWidth() - 470.0f, 40), ImGuiSetCond_FirstUseEver);
        ui::Begin("Lighting", &showLightingSetup, ImGuiWindowFlags_AlwaysAutoResize);

        auto& volumetric = RenderingParams::Volumetric();
        int model = volumetric.enabled ? 1 : 0;

        ui::Text("Model:");
        ui::SameLine();
        ui::RadioButton("Classic", &model, 0);
        ui::SameLine();
        ui::RadioButton("Volumetric", &model, 1);
        volumetric.enabled = model == 1;

        bool changed = false;
        changed |= ui::SliderFloat3("Rotation", value_ptr(lightRotation), -180, 180);
        changed |= ui::DragFloat3("Ambient", value_ptr(lightAmbient), 0.01f, 0, 1);
        changed |= ui::DragFloat3("Diffuse", value_ptr(lightDiffuse), 0.01f, 0, 1);
        changed |= ui::DragFloat("Intensity", &lightIntensity, 0.1f, 0, 32);

        if (changed) { ApplyLight(volume); }

        ui::Separator();

        if (volumetric.enabled)
        {
            ui::SliderFloat("Gradient Shading", &volumetric.gradientShading, 0.0f, 1.0f);
            ui::SliderFloat("Ambient Strength", &volumetric.ambientStrength, 0.0f, 8.0f);
            ui::SliderFloat("Specular", &volumetric.specularStrength, 0.0f, 2.0f);
            ui::SliderFloat("Shininess", &volumetric.shininess, 1.0f, 128.0f);
            ui::Checkbox("Volumetric Shadows", &volumetric.shadows);
            ui::Checkbox("Ambient Occlusion", &volumetric.occlusion);
            ui::SliderFloat("Occlusion Radius", &volumetric.occlusionRadius, 2.0f, 32.0f);
            ui::Checkbox("Indirect Light", &volumetric.indirect);
            ui::SliderFloat("Indirect Strength", &volumetric.indirectStrength, 0.0f, 4.0f);
            ui::Text("Lighting volumes:");
            ui::SameLine();
            ui::RadioButton("Full", &volumetric.volumeDivisor, 1);
            ui::SameLine();
            ui::RadioButton("1/2", &volumetric.volumeDivisor, 2);
            ui::SameLine();
            ui::RadioButton("1/4", &volumetric.volumeDivisor, 4);
            ui::Text("Last rebuild: %.1f ms", volume.getLightingUpdateTime());
        }
        else if (ui::Checkbox("Enable Diffuse Shading", &diffuseShading))
        {
            volume.diffuseShading(diffuseShading);
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