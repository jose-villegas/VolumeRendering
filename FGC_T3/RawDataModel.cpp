#include "RawDataModel.h"
#include <direct.h>

RawDataModel::RawDataModel(void)
{
    isLoaded = false;
    sModelName = (char *)calloc(1024, sizeof(char));
    width = height = numCuts = 1;
}


RawDataModel::~RawDataModel(void)
{
    isLoaded = false;
    free(sModelName);
}

void RawDataModel::Load(const char * pszFilepath, int width, int height, int numCuts)
{
    isLoaded = false;
    FILE * fp;
    size_t size = width * height * numCuts;
    GLubyte * data = new GLubyte[size]; // 8bit

    if (!(fp = fopen(pszFilepath, "rb")))
    {
        std::cout << "Error: opening " << pszFilepath << " file failed: " << std::endl;
        perror("fopen");
        return;
    }
    else
    {
        std:: cout << "OK: open " << pszFilepath << "file successed" << std::endl;
    }

    if (fread(data, sizeof(char), size, fp) != size)
    {
        std::cout << "Error: read .raw file failed" << std::endl;
        fclose(fp);
        return;
    }
    else
    {
        std::cout << "OK: read .raw file successed" << std::endl;
    }

    fclose(fp);
    glGenTextures(1, &_gVolumeTexObj);
    // bind 3D texture target
    glBindTexture(GL_TEXTURE_3D, _gVolumeTexObj);
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
    // Backface Texture
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, &viewport[0]);
    glGenTextures(1, &_backFace2DTex);
    glBindTexture(GL_TEXTURE_2D, _backFace2DTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, viewport[2], viewport[3], 0, GL_RGBA, GL_FLOAT, NULL);
    // Initialize VBO for rendering Volume
    _initVBO();
    isLoaded = true;
}

void RawDataModel::_initVBO()
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

void RawDataModel::render()
{
    if (isLoaded)
    {
        glBindVertexArray(_gVao);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLuint *)NULL);
    }
}
