#pragma once
#include "commons.h"

class RawDataModel
{
    private:
        GLuint _gVolumeTexObj;
        GLuint _backFace2DTex;
        GLuint _gVao;

        void _initVBO();
    public:
        char * sModelName;

        bool isLoaded;
        int width;
        int height;
        int numCuts;

        void Load(const char * pszFilepath, int width, int height, int numCuts);
        void render();
        RawDataModel(void);
        ~RawDataModel(void);
};

