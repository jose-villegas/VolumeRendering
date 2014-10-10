#pragma once
#include "commons.h"
#include "MainData.h"
#include "VolumeShader.h"
#include "GL\glm\glm.hpp"
#include "GL\glm\gtc\matrix_transform.hpp"
#include "GL\glm\gtc\quaternion.hpp"
#include "Camera.h"

class RawDataModel: VolumeShader
{
    private:
        GLuint _tFunc1DTex;
        GLuint _backFace2DTex;
        GLuint _volume3DTex;
        GLuint _depthRenderBuffer;
        GLuint _frameBuffer;
        GLuint _gVao;

        bool _initVBO();
        void _initTransferFunc1DTex();
        bool _init2DBackfaceTex();
        bool _init3DVolumeTex(const char * pszFilepath, int width, int height, int numCuts);
        bool _initFrameBuffer();
        void _renderCubeFace(GLenum gCullFace);
    public:
        char * sModelName;
        bool isLoaded;
        int width;
        int height;
        int numCuts;
        float stepSize;
        GLubyte * data;
        glm::quat rotation;

        void load(const char * pszFilepath, int width, int height, int numCuts);
        void render();
        RawDataModel(void);
        ~RawDataModel(void);
};

