#pragma once
#include "commons.h"

class RawDataModel
{
    private:
        GLuint _gVolumeTexObj;
        GLuint _backFace2DTex;
        GLuint _depthRenderBuffer;
        GLuint _frameBuffer;
        GLuint _gVao;

        bool _initVBO();
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

        void load(const char * pszFilepath, int width, int height, int numCuts);
        void render();
        RawDataModel(void);
        ~RawDataModel(void);
};

