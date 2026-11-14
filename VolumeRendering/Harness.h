#pragma once
#include <string>
#include <cinder/Camera.h>
#include <cinder/CinderGlm.h>

class RaycastVolume;

/**
 * \brief Opt-in command line driver for screenshots, GIF frames and smoke tests.
 * Without arguments the app behaves as an interactive editor and none of this runs.
 *
 *   --preset bonsai|skull|foot     load a bundled volume with a tuned transfer function
 *   --volume <raw> --dims X,Y,Z    load any raw volume, --16bit, --aspect X,Y,Z
 *   --lighting classic|volumetric  lighting model
 *   --off shadows|occlusion|indirect|specular|gradient   turn one volumetric term off, repeatable
 *   --light RX,RY,RZ               light rotation in degrees
 *   --intensity I                  light intensity
 *   --ambient A                    volumetric ambient strength
 *   --rotate RX,RY,RZ              volume rotation in degrees
 *   --threshold MIN,MAX            transfer function iso value range
 *   --camera YAW,PITCH,DISTANCE    orbit camera around the volume, degrees
 *   --size W,H                     window size
 *   --ui | --no-ui                 open the tool windows, or hide every window
 *   --direct                       ray cast straight to the screen, no post-processing
 *   --frames N [--warmup N]        run N frames after warm up then quit
 *   --capture <dir>                save every frame as a png into dir
 *   --orbit D                      orbit the camera D degrees per frame
 *   --light-orbit D                rotate the light D degrees per frame around Y
 *   --compare N                    switch classic / volumetric lighting every N frames
 */
class Harness
{
public:
    static Harness& instance();

    /**
     * \brief Reads the process command line, available before the app settings are applied
     */
    void parseCommandLine();
    void parse(const std::vector<std::string>& args);
    bool isActive() const { return active; }
    bool hidesUi() const { return noUi; }
    glm::ivec2 windowSize() const { return size; }

    /**
     * \brief Loads the volume and applies the requested state, called once after setup
     */
    void start(RaycastVolume& volume, ci::CameraPersp& camera);
    /**
     * \brief Advances animations, called every update
     */
    void update(RaycastVolume& volume, ci::CameraPersp& camera);
    /**
     * \brief Saves the frame and quits when done, called after the ui is rendered
     */
    void postDraw();
private:
    void applyPreset(RaycastVolume& volume);
    void placeCamera(ci::CameraPersp& camera) const;

    bool active = false;
    bool noUi = false;
    bool direct = false;
    bool openUi = false;
    std::string preset;
    std::string volumePath;
    glm::ivec3 dims{256};
    glm::vec3 aspect{1};
    bool is16Bits = false;
    int lighting = -1;
    bool hasLight = false;
    glm::vec3 light{0};
    glm::vec3 cameraOrbit{0, 0, 4};
    bool hasCamera = false;
    glm::ivec2 size{0};
    int frames = 0;
    int warmup = 20;
    std::string captureDir;
    float orbit = 0;
    float lightOrbit = 0;
    int compare = 0;
    glm::vec3 rotation{0};
    bool hasRotation = false;
    glm::ivec2 threshold{0};
    float intensity = 1.0f;

    int frame = 0;
    double captureStart = 0;
};
