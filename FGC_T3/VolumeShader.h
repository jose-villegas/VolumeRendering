#pragma once
#include "commons.h"

class VolumeShader {
    public:
        VolumeShader(void);
        ~VolumeShader(void);
    protected:
        GLuint bfVertHandle;
        GLuint bfFragHandle;
        GLuint rcVertHandle;
        GLuint rcFragHandle;
        GLuint programHandle;

        void initShaders();
        void enableShader();
        void disableShader();
        void rcSetUinforms(float stepSize, GLuint tFunc1DTex, GLuint backFace2DTex, GLuint volume3DTex);
        void linkShaderBackface();
        void linkShaderRayCasting();
        GLint checkShaderLinkStatus(GLuint pgmHandle);
    private:
        GLint screenSizeLoc;
        GLint stepSizeLoc;
        GLint transferFuncLoc;
        GLint backFaceLoc;
        GLint volumeLoc;

        GLuint _initShaderObj(const GLchar * srcfile, GLenum shaderType);
        GLuint _createShaderPgm();
        GLboolean _compileCheck(GLuint shader);
        void _linkShader(GLuint shaderPgm, GLuint newVertHandle, GLuint newFragHandle);
};

