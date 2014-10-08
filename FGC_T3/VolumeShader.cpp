#include "VolumeShader.h"
#include "MainData.h"


VolumeShader::VolumeShader(void)
{
    screenSizeLoc = -1;
    stepSizeLoc = -1;
    transferFuncLoc = -1;
    backFaceLoc = -1;
    volumeLoc = -1;
}


VolumeShader::~VolumeShader(void)
{
}

void VolumeShader::initShaders()
{
    // vertex shader object for first pass
    bfVertHandle = initShaderObj("Shaders/backface.vert", GL_VERTEX_SHADER);
    // fragment shader object for first pass
    bfFragHandle = initShaderObj("Shaders/backface.frag", GL_FRAGMENT_SHADER);
    // vertex shader object for second pass
    rcVertHandle = initShaderObj("Shaders/raycasting.vert", GL_VERTEX_SHADER);
    // fragment shader object for second pass
    rcFragHandle = initShaderObj("Shaders/raycasting.frag", GL_FRAGMENT_SHADER);
    // create the shader program , use it in an appropriate time
    programHandle = createShaderPgm();
}

GLuint VolumeShader::initShaderObj(const GLchar * srcfile, GLenum shaderType)
{
    std::ifstream inFile(srcfile, std::ifstream::in);

    // use assert?
    if (!inFile)
    {
        std::cerr << "Error openning file: " << srcfile << std::endl;
        exit(EXIT_FAILURE);
    }

    const int MAX_CNT = 10000;
    GLchar * shaderCode = (GLchar *) calloc(MAX_CNT, sizeof(GLchar));
    inFile.read(shaderCode, MAX_CNT);

    if (inFile.eof())
    {
        size_t bytecnt = inFile.gcount();
        *(shaderCode + bytecnt) = '\0';
    }
    else if (inFile.fail())
    {
        std::cout << srcfile << "read failed " << std::endl;
    }
    else
    {
        std::cout << srcfile << "is too large" << std::endl;
    }

    // create the shader Object
    GLuint shader = glCreateShader(shaderType);

    if (0 == shader)
    {
        std::cerr << "Error creating vertex shader." << std::endl;
    }

    // cout << shaderCode << endl;
    // cout << endl;
    const GLchar * codeArray[] = {shaderCode};
    glShaderSource(shader, 1, codeArray, NULL);
    free(shaderCode);
    // compile the shader
    glCompileShader(shader);

    if (GL_FALSE == compileCheck(shader))
    {
        std::cerr << "shader compilation failed" << std::endl;
    }

    return shader;
}

GLboolean VolumeShader::compileCheck(GLuint shader)
{
    GLint err;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &err);

    if (GL_FALSE == err)
    {
        GLint logLen;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);

        if (logLen > 0)
        {
            char * log = (char *)malloc(logLen);
            GLsizei written;
            glGetShaderInfoLog(shader, logLen, &written, log);
            std::cerr << "Shader log: " << log << std::endl;
            free(log);
        }
    }

    return err;
}

GLuint VolumeShader::createShaderPgm()
{
    // Create the shader program
    GLuint programHandle = glCreateProgram();

    if (0 == programHandle)
    {
        std::cerr << "Error create shader program" << std::endl;
        exit(EXIT_FAILURE);
    }

    return programHandle;
}

void VolumeShader::linkShader(GLuint shaderPgm, GLuint newVertHandle, GLuint newFragHandle)
{
    const GLsizei maxCount = 2;
    GLsizei count;
    GLuint shaders[maxCount];
    glGetAttachedShaders(shaderPgm, maxCount, &count, shaders);
    // cout << "get VertHandle: " << shaders[0] << endl;
    // cout << "get FragHandle: " << shaders[1] << endl;

    for (int i = 0; i < count; i++)
    {
        glDetachShader(shaderPgm, shaders[i]);
    }

    // Bind index 0 to the shader input variable "VerPos"
    glBindAttribLocation(shaderPgm, 0, "VerPos");
    // Bind index 1 to the shader input variable "VerClr"
    glBindAttribLocation(shaderPgm, 1, "VerClr");
    glAttachShader(shaderPgm, newVertHandle);
    glAttachShader(shaderPgm, newFragHandle);
    glLinkProgram(shaderPgm);

    if (GL_FALSE == checkShaderLinkStatus(shaderPgm))
    {
        std::cerr << "Failed to relink shader program!" << std::endl;
        exit(EXIT_FAILURE);
    }
}

GLint VolumeShader::checkShaderLinkStatus(GLuint pgmHandle)
{
    GLint status;
    glGetProgramiv(pgmHandle, GL_LINK_STATUS, &status);

    if (GL_FALSE == status)
    {
        GLint logLen;
        glGetProgramiv(pgmHandle, GL_INFO_LOG_LENGTH, &logLen);

        if (logLen > 0)
        {
            GLchar * log = (GLchar *)malloc(logLen);
            GLsizei written;
            glGetProgramInfoLog(pgmHandle, logLen, &written, log);
            std::cerr << "Program log: " << log << std::endl;
        }
    }

    return status;
}

void VolumeShader::linkShaderBackface()
{
    linkShader(programHandle, bfVertHandle, bfFragHandle);
}

void VolumeShader::linkShaderRayCasting()
{
    linkShader(programHandle, rcVertHandle, rcFragHandle);
}

void VolumeShader::enableShader()
{
    glUseProgram(programHandle);
}

void VolumeShader::disableShader()
{
    glUseProgram(0);
}

void VolumeShader::rcSetUinforms(float stepSize, GLuint tFunc1DTex, GLuint backFace2DTex, GLuint volume3DTex)
{
    // setting uniforms such as
    // ScreenSize
    // StepSize
    // TransferFunc
    // ExitPoints i.e. the backface, the backface hold the ExitPoints of ray casting
    // VolumeTex the texture that hold the volume data i.e. head256.raw
    if (screenSizeLoc == -1) { screenSizeLoc = glGetUniformLocation(programHandle, "ScreenSize"); }

    if (stepSizeLoc == -1) { stepSizeLoc = glGetUniformLocation(programHandle, "StepSize"); }

    if (transferFuncLoc == -1) { transferFuncLoc = glGetUniformLocation(programHandle, "TransferFunc"); }

    if (backFaceLoc == -1) { backFaceLoc = glGetUniformLocation(programHandle, "ExitPoints"); }

    if (volumeLoc == -1) { volumeLoc = glGetUniformLocation(programHandle, "VolumeTex"); }

    if (screenSizeLoc >= 0)
    {
        glUniform2f(screenSizeLoc, (float)MainData::rootWindow->getSize().x, (float)MainData::rootWindow->getSize().y);
    }
    else
    {
        std::cout << "ScreenSize" << "is not bind to the uniform" << std::endl;
    }

    if (stepSizeLoc >= 0)
    {
        glUniform1f(stepSizeLoc, stepSize);
    }
    else
    {
        std::cout << "StepSize" << "is not bind to the uniform" << std::endl;
    }

    if (transferFuncLoc >= 0)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_1D, tFunc1DTex);
        glUniform1i(transferFuncLoc, 0);
    }
    else
    {
        std::cout << "TransferFunc" << "is not bind to the uniform" << std::endl;
    }

    if (backFaceLoc >= 0)
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, backFace2DTex);
        glUniform1i(backFaceLoc, 1);
    }
    else
    {
        std::cout << "ExitPoints" << "is not bind to the uniform" << std::endl;
    }

    if (volumeLoc >= 0)
    {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_3D, volume3DTex);
        glUniform1i(volumeLoc, 2);
    }
    else
    {
        std::cout << "VolumeTex" << "is not bind to the uniform" << std::endl;
    }
}
