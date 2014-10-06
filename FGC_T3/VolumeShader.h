#pragma once
#include "commons.h"

class VolumeShader
{
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
    private:
        GLuint initShaderObj(const GLchar * srcfile, GLenum shaderType);
        GLboolean compileCheck(GLuint shader);
        GLuint createShaderPgm();
};

