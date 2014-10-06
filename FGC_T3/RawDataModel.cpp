#include "RawDataModel.h"
#include "GL\glm\glm.hpp"
#include "GL\glm\gtc\matrix_transform.hpp"

RawDataModel::RawDataModel(void)
{
    isLoaded = false;
    sModelName = (char *)calloc(1024, sizeof(char));
    width = height = numCuts = 1;
    stepSize = 0.001f;
    initShaders();
    _initTransferFunc1DTex();
}


RawDataModel::~RawDataModel(void)
{
    isLoaded = false;
    free(sModelName);
}

void RawDataModel::load(const char * pszFilepath, int width, int height, int numCuts)
{
    isLoaded = false;

    // Initialize VBO for rendering Volume
    if (!_initVBO()) { return; }

    // Load Volume data on 3D texture
    if (!_init3DVolumeTex(pszFilepath, width, height, numCuts)) { return; }

    // Initialize texture for backface
    if (!_init2DBackfaceTex()) { return; }

    // Initialize FBO
    if (!_initFrameBuffer()) { return; }

    // Success
    this->width = width;
    this->height = height;
    this->numCuts = numCuts;
    memcpy(sModelName, pszFilepath, 1024);
    isLoaded = true;
}

bool RawDataModel::_initVBO()
{
    try
    {
        GLfloat vertices[24] =
        {
            0.0, 0.0, 0.0,
            0.0, 0.0, 1.0,
            0.0, 1.0, 0.0,
            0.0, 1.0, 1.0,
            1.0, 0.0, 0.0,
            1.0, 0.0, 1.0,
            1.0, 1.0, 0.0,
            1.0, 1.0, 1.0
        };
        // Counter clockwise bounding box faces
        GLuint indices[36] =
        {
            1, 5, 7,
            7, 3, 1,
            0, 2, 6,
            6, 4, 0,
            0, 1, 3,
            3, 2, 0,
            7, 5, 4,
            4, 6, 7,
            2, 3, 7,
            7, 6, 2,
            1, 0, 4,
            4, 5, 1
        };
        GLuint gbo[2];
        glGenBuffers(2, gbo);
        GLuint vertexdat = gbo[0];
        GLuint indexdat = gbo[1];
        glBindBuffer(GL_ARRAY_BUFFER, vertexdat);
        glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
        // used in glDrawElement()
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexdat);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(GLuint), indices, GL_STATIC_DRAW);
        // Setup Vertex Array Object
        glGenVertexArrays(1, &_gVao);
        // vao like a closure binding 3 buffer object: verlocdat vercoldat and veridxdat
        glBindVertexArray(_gVao);
        glEnableVertexAttribArray(0); // for vertexloc
        glEnableVertexAttribArray(1); // for vertexcol
        // the vertex location is the same as the vertex color
        glBindBuffer(GL_ARRAY_BUFFER, vertexdat);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexdat);
    }
    catch (...)
    {
        return false;
    }

    return true;
}

void RawDataModel::render()
{
    if (isLoaded)
    {
        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _frameBuffer);
        glViewport(0, 0, MainData::rootWindow->getSize().x, MainData::rootWindow->getSize().y);
        linkShaderBackface();
        enableShader();
        _renderCubeFace(GL_FRONT);
        disableShader();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, MainData::rootWindow->getSize().x, MainData::rootWindow->getSize().y);
        linkShaderRayCasting();
        enableShader();
        rcSetUinforms(stepSize, _tFunc1DTex, _backFace2DTex, _volume3DTex);
        _renderCubeFace(GL_BACK);
        disableShader();
    }
}

bool RawDataModel::_init2DBackfaceTex()
{
    try
    {
        // Backface Texture
        glGenTextures(1, &_backFace2DTex);
        glBindTexture(GL_TEXTURE_2D, _backFace2DTex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, MainData::rootWindow->getSize().x, MainData::rootWindow->getSize().y, 0, GL_RGBA, GL_FLOAT, NULL);
    }
    catch (...)
    {
        return false;
    }

    return true;
}

bool RawDataModel::_init3DVolumeTex(const char * pszFilepath, int width, int height, int numCuts)
{
    FILE * fp;
    size_t size = width * height * numCuts;
    GLubyte * data = new GLubyte[size]; // 8bit

    if (!(fp = fopen(pszFilepath, "rb")))
    {
        std::cout << "Error: opening " << pszFilepath << " file failed: " << std::endl;
        perror("fopen");
        return false;
    }
    else
    {
        std:: cout << "OK: opening " << pszFilepath << " file successed" << std::endl;
    }

    if (fread(data, sizeof(char), size, fp) != size)
    {
        std::cout << "Error: reading " << pszFilepath << " file failed" << std::endl;
        fclose(fp);
        return false;
    }
    else
    {
        std::cout << "OK: reading " << pszFilepath << " file successed" << std::endl;
    }

    fclose(fp);
    glGenTextures(1, &_volume3DTex);
    // bind 3D texture target
    glBindTexture(GL_TEXTURE_3D, _volume3DTex);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    // pixel transfer happens here from client to OpenGL server
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_INTENSITY, width, height, numCuts, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
    delete []data;
    std::cout << "volume texture created" << std::endl;
    return true;
}

bool RawDataModel::_initFrameBuffer()
{
    glGenRenderbuffers(1, &_depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, MainData::rootWindow->getSize().x, MainData::rootWindow->getSize().y);
    // attach the texture and the depth buffer to the framebuffer
    glGenFramebuffers(1, &_frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _backFace2DTex, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);
    GLenum complete = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (complete != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "framebuffer is not complete" << std::endl;
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void RawDataModel::_renderCubeFace(GLenum gCullFace)
{
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //  transform the box
    glm::mat4 projection = glm::perspective(90.0f, (GLfloat)MainData::rootWindow->getSize().x / MainData::rootWindow->getSize().y, 1.0f, 500.f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, -3.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, MainData::mainClock->getElapsedTime().asSeconds() * 50, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, MainData::mainClock->getElapsedTime().asSeconds() * 30, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, MainData::mainClock->getElapsedTime().asSeconds() * 90, glm::vec3(1.0f, 0.0f, 1.0f));
    // to make the "head256.raw" i.e. the volume data stand up.
    model = glm::rotate(model, 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    // notice the multiplication order: reverse order of transform
    glm::mat4 mvp = projection * view * model;
    GLuint mvpIdx = glGetUniformLocation(programHandle, "MVP");

    if (mvpIdx >= 0)
    {
        glUniformMatrix4fv(mvpIdx, 1, GL_FALSE, &mvp[0][0]);
    }
    else
    {
        std::cerr << "can't get the MVP" << std::endl;
    }

    glEnable(GL_CULL_FACE);
    glCullFace(gCullFace);
    glBindVertexArray(_gVao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLuint *)NULL);
    glDisable(GL_CULL_FACE);
}

void RawDataModel::_initTransferFunc1DTex()
{
    std::ifstream inFile("tff.dat", std::ifstream::in);

    if (!inFile)
    {
        std::cerr << "Error openning file: " << "tff.dat" << std::endl;
        exit(EXIT_FAILURE);
    }

    const int MAX_CNT = 10000;
    GLubyte * tff = (GLubyte *) calloc(MAX_CNT, sizeof(GLubyte));
    inFile.read(reinterpret_cast<char *>(tff), MAX_CNT);

    if (inFile.eof())
    {
        size_t bytecnt = inFile.gcount();
        *(tff + bytecnt) = '\0';
        std::cout << "bytecnt " << bytecnt << std::endl;
    }
    else if (inFile.fail())
    {
        std::cout << "tff.dat" << "read failed " << std::endl;
    }
    else
    {
        std::cout << "tff.dat" << "is too large" << std::endl;
    }

    GLuint tff1DTex;
    glGenTextures(1, &tff1DTex);
    glBindTexture(GL_TEXTURE_1D, tff1DTex);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, tff);
    free(tff);
}
