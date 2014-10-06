#include "VolumeShader.h"


VolumeShader::VolumeShader(void)
{
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
