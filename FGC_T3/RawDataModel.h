#pragma once
#include "commons.h"
#include "MainData.h"
#include "VolumeShader.h"
#include "GL\glm\glm.hpp"
#include "GL\glm\gtc\matrix_transform.hpp"
#include "GL\glm\gtc\quaternion.hpp"
#include "Camera.h"

class RawDataModel: VolumeShader {
    private:
        GLuint _backFace2DTex;
        GLuint _depthRenderBuffer;
        GLuint _frameBuffer;
        GLuint _gVao;
        GLuint _tFunc1DTex;
        GLuint _volume3DTex;
        glm::vec3 _normalizedBBox;
        int _heightP;
        int _numCutsP;
        int _widthP;

        bool _initVBO();
        bool _init2DBackfaceTex();
        bool _init3DVolumeTex(const char * pszFilepath, int width, int height, int numCuts);
        bool _initFrameBuffer();
        void _renderCubeFace(GLenum gCullFace);
    public:
        GLubyte * data;
        GLubyte transferFunc[256][4];
        glm::quat rotation;
        bool isLoaded;
        char * sModelName;
        float stepSize;
        int height;
        int numCuts;
        int width;

        void load(const char * pszFilepath, int width, int height, int numCuts);
        void render();
        void updateTransferFunc1DTex();
        RawDataModel(void);
        ~RawDataModel(void);
};

