#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "CinderImGui.h"
#include "RaycastVolume.h"
#include "PostProcess.h"
#include "VolumeRenderingAppUi.h"
#include "RenderingParams.h"

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
    RaycastVolume volume;
    PostProcess postProcess;
    CameraPersp camera;
    CameraPersp initialCamera;
    float dragPivotDistance{0.0f};
};

void VolumeRenderingApp::prepareSettings(Settings* settings)
{
    settings->setWindowSize(1280, 720);
}

void VolumeRenderingApp::setup()
{
    auto options = ImGui::Options();
    options.font("fonts/DroidSans.ttf", 16);
    ui::initialize(options);
    // set camera initial setup
    camera.lookAt(vec3(0, 0, -4), vec3(0), vec3(0, 1, 0));
}

void VolumeRenderingApp::update()
{
    VolumeRenderingAppUi::DrawUi(volume);
}

void VolumeRenderingApp::draw()
{
    gl::clear();
    // volume raycasting
    {
        volume.setPosition(-volume.centerPoint());
        volume.drawVolume(camera, VolumeRenderingAppUi::PostProcessingEnabled());
    }
    // post-process
    if (VolumeRenderingAppUi::PostProcessingEnabled())
    {
        postProcess.toneMapping(volume.getColorTexture());

        // anti aliasing
        if(RenderingParams::FXAAEnabled())
        {
            postProcess.fxAA();
        }
        else
        {
            postProcess.displayTexture();
        }
    }
}

void VolumeRenderingApp::resize()
{
    camera.setAspectRatio(getWindowAspectRatio());
    // update frame buffers
    volume.resizeFbos();
    postProcess.resizeFbos();
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
        dragStart = event.getPos();
        initialCamera = camera;
        dragPivotDistance = camera.getPivotDistance();
    }
}

CINDER_APP (VolumeRenderingApp, RendererGl, &VolumeRenderingApp::prepareSettings)
