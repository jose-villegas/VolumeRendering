#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "CinderImGui.h"
#include "RaycastVolume.h"
#include "PostProcess.h"
#include "VolumeRenderingAppUi.h"
#include "RenderingParams.h"
#include "Harness.h"

using namespace ci;
using namespace app;

class VolumeRenderingApp : public App
{
public:
    static void prepareSettings(Settings* settings);
    void setup() override;
    void update() override;
    void draw() override;
    void resize() override;
    void mouseWheel(MouseEvent event) override;
    void mouseDrag(MouseEvent event) override;
    void mouseDown(MouseEvent event) override;
private:
    vec2 dragStart;
    // created in setup, once the window's OpenGL context is current
    std::unique_ptr<RaycastVolume> volumePtr;
    std::unique_ptr<PostProcess> postProcess;
    CameraPersp camera;
    CameraPersp initialCamera;
    float dragPivotDistance{0.0f};
};

void VolumeRenderingApp::prepareSettings(Settings* settings)
{
    settings->setWindowSize(1280, 720);
    // opt-in command line driver for captures, does nothing without arguments
    auto& harness = Harness::instance();
    harness.parseCommandLine();

    if (harness.windowSize().x > 0) settings->setWindowSize(harness.windowSize());
}

void VolumeRenderingApp::setup()
{
    auto options = ImGui::Options();
    options.font("fonts/DroidSans.ttf", 16);
    ui::initialize(options);

    // captures always start from the default window layout
    if (Harness::instance().isActive()) ImGui::GetIO().IniFilename = nullptr;
    // set camera initial setup
    camera.lookAt(vec3(0, 0, -4), vec3(0), vec3(0, 1, 0));
    volumePtr = std::make_unique<RaycastVolume>();
    postProcess = std::make_unique<PostProcess>();
    VolumeRenderingAppUi::Initialize(*volumePtr);
    // connected after the ui so captures include it
    getWindow()->getSignalPostDraw().connect([] { Harness::instance().postDraw(); });
    Harness::instance().start(*volumePtr, camera);
}

void VolumeRenderingApp::update()
{
    if (!Harness::instance().hidesUi()) VolumeRenderingAppUi::DrawUi(*volumePtr);

    Harness::instance().update(*volumePtr, camera);
}

void VolumeRenderingApp::draw()
{
    gl::clear();
    // volume raycasting
    {
        volumePtr->setPosition(-volumePtr->centerPoint());
        volumePtr->drawVolume(camera, VolumeRenderingAppUi::PostProcessingEnabled());
    }
    // post-process
    if (VolumeRenderingAppUi::PostProcessingEnabled() && volumePtr->hasVolume())
    {
        postProcess->toneMapping(volumePtr->getColorTexture());

        // anti aliasing
        if(RenderingParams::FXAAEnabled())
        {
            postProcess->fxAA();
        }
        else
        {
            postProcess->displayTexture();
        }
    }
}

void VolumeRenderingApp::resize()
{
    camera.setAspectRatio(getWindowAspectRatio());
    // update frame buffers
    if (volumePtr) volumePtr->resizeFbos();
    if (postProcess) postProcess->resizeFbos();
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
        float deltaX = (dragCurrent.x - dragStart.x) / -100.0f;
        float deltaY = (dragCurrent.y - dragStart.y) / 100.0f;
        vec3 viewDirection = normalize(initialCamera.getViewDirection());
        bool invertMotion = (initialCamera.getOrientation() * initialCamera.getWorldUp()).y < 0.0f;

        vec3 rightDirection = normalize(cross(initialCamera.getWorldUp(), viewDirection));

        if (invertMotion)
        {
            deltaX = -deltaX;
            deltaY = -deltaY;
        }

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
        dragStart = event.getPos();
        initialCamera = camera;
        dragPivotDistance = camera.getPivotDistance();
    }
}

CINDER_APP (VolumeRenderingApp, RendererGl, &VolumeRenderingApp::prepareSettings)
