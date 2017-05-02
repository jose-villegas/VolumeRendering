#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "CinderImGui.h"
#include "RaycastVolume.h"
#include <cinder/Log.h>
#include "TransferFunctionUi.h"

using namespace ci;
using namespace app;

class VolumeRenderingApp : public App
{
public:
    static void prepareSettings(Settings* settings);
    void setup() override;
    void volumeParametersPopupUi(fs::path& path);
    void renderingOptionsUi(bool& showRendering);
    void drawUi();
    void update() override;
    void draw() override;
    void mouseWheel(MouseEvent event) override;
    void mouseDrag(MouseEvent event) override;
    void mouseDown(MouseEvent event) override;
private:
    std::shared_ptr<TransferFunctionUi> transferFunctionUi;
    quat modelRotation;
    vec2 mouseDragStart;
    RaycastVolume volume;
    CameraPersp camera;
    CameraPersp initialCamera;
    float dragPivotDistance{0.0f};
};

void VolumeRenderingApp::prepareSettings(Settings* settings)
{
    settings->setResizable(false);
    settings->setWindowSize(1280, 720);
}

void VolumeRenderingApp::setup()
{
    auto options = ImGui::Options();
    options.font("fonts/DroidSans.ttf", 16);
    ui::initialize(options);
    // pass transfer function ref
    transferFunctionUi = std::make_shared<TransferFunctionUi>();
    volume.setTransferFunction(transferFunctionUi);
    // set camera initial setup
    camera.lookAt(vec3(0, 0, -4), vec3(0), vec3(0, 1, 0));
}

void VolumeRenderingApp::volumeParametersPopupUi(fs::path& path)
{
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
            volume.loadFromFile(slices, ratios, path.string(), bits == 1);
            ui::CloseCurrentPopup();
        }

        ui::EndPopup();
    }
}

void VolumeRenderingApp::renderingOptionsUi(bool& showRendering)
{
    if (showRendering)
    {
        ui::Begin("Rendering", &showRendering, ImGuiWindowFlags_AlwaysAutoResize);
        static float stepScale = volume.getStepScale();
        static vec3 aspectRatios = volume.getAspectRatios();
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

        ui::End();
    }
}

void VolumeRenderingApp::drawUi()
{
    static bool loadNewVolume = false;
    static bool showVolumeOptions = false;
    static bool showRendering = false;
    static bool showTransferFunction = false;
    static bool showRotation = false;
    static bool showLightingSetup;
    static fs::path path;

    // menu bar on top
    {
        ui::ScopedMainMenuBar menuBar;

        if (ui::BeginMenu("File"))
        {
            if (ui::MenuItem("Open"))
            {
                path = getOpenFilePath(path, {"raw"});

                if (!path.empty())
                {
                    loadNewVolume = true;
                }
            }
            ui::EndMenu();
        }

        if (ui::BeginMenu("Volume", showVolumeOptions))
        {
            ui::MenuItem("Rotate", nullptr, &showRotation);
            ui::MenuItem("Rendering", nullptr, &showRendering);
            ui::MenuItem("Lighting", nullptr, &showLightingSetup);
            ui::MenuItem("Transfer Function", nullptr, &showTransferFunction);

            ui::EndMenu();
        }
    }

    // open modal dialog for volume parameters
    if (loadNewVolume)
    {
        ui::OpenPopup("Volume Parameters");
        showVolumeOptions = true;
    }

    volumeParametersPopupUi(path);

    if (showRotation)
    {
        static vec3 angles = vec3(0);
        bool changed = false;
        ui::Begin("Rotate", &showRotation, ImGuiWindowFlags_AlwaysAutoResize);
        changed |= ui::SliderAngle("X", &angles.x);
        changed |= ui::SliderAngle("Y", &angles.y);
        changed |= ui::SliderAngle("Z", &angles.z);

        if (changed)
        {
            modelRotation = toQuat(glm::eulerAngleXYZ(angles.x, angles.y, angles.z));
        }

        ui::End();
    }

    if (showLightingSetup)
    {
        static Light light = volume.getLight();
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

        if (changed)
        {
            vec3 rads = radians(rotation);
            light.direction = vec3(0, 0, 1) * mat3(glm::eulerAngleXYZ(rads.x, rads.y, rads.z));
            volume.setLight(light.direction, light.ambient, light.diffuse);
        }

        ui::End();
    }

    // volume controls
    renderingOptionsUi(showRendering);
    // transfer function ui
    transferFunctionUi->drawUi(showTransferFunction, volume);
    // refresh flags
    loadNewVolume = false;
}

void VolumeRenderingApp::update()
{
    drawUi();
}

void VolumeRenderingApp::draw()
{
    gl::clear();
    gl::setMatrices(camera);

    // volume raycasting
    {
        gl::ScopedModelMatrix modelMatrix;
        // rotate around center
        gl::rotate(modelRotation);
        // center to origin
        gl::translate(-volume.centerPoint());

        volume.drawVolume();
    }
}

void VolumeRenderingApp::mouseWheel(MouseEvent event)
{
    float increment = event.getWheelIncrement();
    float multiplier = powf(1.2f, increment);
    // move camera
    vec3 translate = camera.getViewDirection() * (camera.getPivotDistance() * (1.0f - multiplier));
    camera.setEyePoint(camera.getEyePoint() + translate);
    camera.setPivotDistance(camera.getPivotDistance() * multiplier);
}

void VolumeRenderingApp::mouseDrag(MouseEvent event)
{
    if (event.isLeftDown())
    {
        vec2 dragCurrent = event.getPos();
        float deltaX = (dragCurrent.x - mouseDragStart.x) / -100.0f;
        float deltaY = (dragCurrent.y - mouseDragStart.y) / 100.0f;
        vec3 viewDirection = normalize(initialCamera.getViewDirection());
        bool invertMotion = (initialCamera.getOrientation() * initialCamera.getWorldUp()).y < 0.0f;

        vec3 rightDirection = normalize(cross(initialCamera.getWorldUp(), viewDirection));

        if (invertMotion)
        {
            deltaX = -deltaX;
            deltaY = -deltaY;
        }

        console() << deltaX << ", " << deltaY << std::endl;

        vec3 rotated = angleAxis(deltaY, rightDirection) * (-initialCamera.getViewDirection() * dragPivotDistance);
        rotated = angleAxis(deltaX, initialCamera.getWorldUp()) * rotated;

        camera.setEyePoint(initialCamera.getEyePoint() + initialCamera.getViewDirection() * dragPivotDistance + rotated);
        camera.setOrientation(angleAxis(deltaX, initialCamera.getWorldUp()) * angleAxis(deltaY, rightDirection) * initialCamera.getOrientation());
    }
}

void VolumeRenderingApp::mouseDown(MouseEvent event)
{
    if (event.isLeftDown())
    {
        mouseDragStart = event.getPos();
        initialCamera = camera;
        dragPivotDistance = camera.getPivotDistance();
    }
}

CINDER_APP (VolumeRenderingApp, RendererGl, &VolumeRenderingApp::prepareSettings)
