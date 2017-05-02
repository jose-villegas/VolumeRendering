#include "RaycastVolume.h"
#include <cinder/app/AppBase.h>
#include <cinder/Log.h>
#include "TransferFunction.h"
using namespace ci;
using namespace glm;

RaycastVolume::RaycastVolume() : aspectRatios(1), scaleFactor(vec3(1)), stepScale(1), enableDiffuseShading(true)
{
    // positions shader
    positionsShader = gl::GlslProg::create(gl::GlslProg::Format()
        .vertex(loadFile("shaders/positions.vert"))
        .fragment(loadFile("shaders/positions.frag")));
    // raycast shader
    raycastShader = gl::GlslProg::create(gl::GlslProg::Format()
        .vertex(loadFile("shaders/raycast.vert"))
        .fragment(loadFile("shaders/raycast.frag")));
    // histogram calculation 
    histogramCompute = gl::GlslProg::create(gl::GlslProg::Format()
        .compute(loadFile("shaders/histogram.comp")));
    // gradient computation
    gradientsCompute = gl::GlslProg::create(gl::GlslProg::Format()
        .compute(loadFile("shaders/gradients.comp")));
    smoothGradientsCompute = gl::GlslProg::create(gl::GlslProg::Format()
        .compute(loadFile("shaders/smooth_gradients.comp")));
    // noise texture to reduce volume banding artifacts
    noiseTexture = gl::Texture2d::create(loadImage("images/noise.png"), gl::Texture2d::Format()
                                         .wrapS(GL_REPEAT)
                                         .wrapT(GL_REPEAT)
                                         .magFilter(GL_NEAREST)
                                         .minFilter(GL_NEAREST));
    // create clockwise bbox for volume rendering
    createCubeVbo();
    // create frame buffer object
    createFbos();
}

RaycastVolume::~RaycastVolume() {}

vec3 RaycastVolume::centerPoint() const
{
    return vec3(0.5) * scaleFactor;
}

const float &RaycastVolume::getStepScale() const
{
    return stepScale;
}

void RaycastVolume::setStepScale(const float& value)
{
    stepScale = max(value, 0.1f);
}

const vec3 &RaycastVolume::getAspectRatios() const
{
    return aspectRatios;
}

void RaycastVolume::setAspectratios(const vec3& value)
{
    aspectRatios = max(value, vec3(0));
    // volume scale
    scaleFactor = vec3(1) / ((vec3(1) * maxSize) / (dimensions * aspectRatios));
}

void RaycastVolume::createCubeVbo()
{
    GLfloat vertices[24] =
    {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f
    };

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

    // setup counter clock wise bbox data
    verticesBuffer = gl::BufferObj::create(GL_ARRAY_BUFFER, 24 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    indicesBuffer = gl::BufferObj::create(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(GLuint), indices, GL_STATIC_DRAW);
    // setup vao for object
    vertexArrayObject = gl::Vao::create();
    vertexArrayObject->bind();
    // vertex attributes locations
    gl::enableVertexAttribArray(0);
    // bind and link data
    verticesBuffer->bind();
    gl::vertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, static_cast<GLfloat *>(nullptr));
    indicesBuffer->bind();
}

void RaycastVolume::readVolumeFromFile8(const std::string filepath)
{
    // open file and read
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);

    // error opening given filepath
    if (!file.good()) return;

    // determine volume size
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    // reserve space for data
    std::vector<uint8_t> buffer(size);

    if (file.read(reinterpret_cast<char*>(buffer.data()), size))
    {
        // create 3D texture
        auto format = gl::Texture3d::Format().magFilter(GL_LINEAR)
                                             .minFilter(GL_LINEAR)
                                             .wrapS(GL_CLAMP_TO_BORDER)
                                             .wrapR(GL_CLAMP_TO_BORDER)
                                             .wrapT(GL_CLAMP_TO_BORDER);
        format.setDataType(GL_UNSIGNED_BYTE);
        format.setInternalFormat(GL_RED);
        format.setSwizzleMask(GL_RED, GL_RED, GL_RED, GL_RED);
        volumeTexture = gl::Texture3d::create(buffer.data(), GL_RED, dimensions.x, dimensions.y, dimensions.z, format);
        // finally has drawable data
        isDrawable = true;
    }

    file.close();
}

void RaycastVolume::readVolumeFromFile16(const std::string filepath)
{
    // open file and read
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);

    // error opening given filepath
    if (!file.good()) return;

    // determine volume size
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    // reserve space for data
    std::vector<uint16_t> buffer(size);

    if (file.read(reinterpret_cast<char*>(buffer.data()), size))
    {
        // create 3D texture
        auto format = gl::Texture3d::Format().magFilter(GL_LINEAR)
                                             .minFilter(GL_LINEAR)
                                             .wrapS(GL_CLAMP_TO_BORDER)
                                             .wrapR(GL_CLAMP_TO_BORDER)
                                             .wrapT(GL_CLAMP_TO_BORDER);
        format.setDataType(GL_UNSIGNED_SHORT);
        format.setInternalFormat(GL_RED);
        format.setSwizzleMask(GL_RED, GL_RED, GL_RED, GL_RED);
        volumeTexture = gl::Texture3d::create(buffer.data(), GL_RED, dimensions.x, dimensions.y, dimensions.z, format);
        // finally has drawable data
        isDrawable = true;
    }

    file.close();
}

void RaycastVolume::createFbos()
{
    gl::Fbo::Format format = gl::Fbo::Format();
    format.colorTexture(gl::Texture::Format().wrapS(GL_REPEAT)
                                             .wrapT(GL_REPEAT)
                                             .minFilter(GL_NEAREST)
                                             .magFilter(GL_NEAREST)
                                             .internalFormat(GL_RGBA16F)
                                             .dataType(GL_FLOAT));
    format.depthBuffer(GL_DEPTH_COMPONENT16);

    try
    {
        frontDepth = gl::Renderbuffer::create(app::getWindowWidth(), app::getWindowHeight(), GL_DEPTH_COMPONENT16);
        frontFbo = gl::Fbo::create(app::getWindowWidth(), app::getWindowHeight(), format);
        backDepth = gl::Renderbuffer::create(app::getWindowWidth(), app::getWindowHeight(), GL_DEPTH_COMPONENT16);
        backFbo = gl::Fbo::create(app::getWindowWidth(), app::getWindowHeight(), format);
    }
    catch (const Exception& e)
    {
        CI_LOG_EXCEPTION("Fbo/Renderbuffer create", e);
    }

    CI_CHECK_GL();
}

void RaycastVolume::loadFromFile(const vec3& dimensions, const vec3& ratios, const std::string filepath, bool is16Bits)
{
    // compute step size and number of iterations for the given volume dimensions
    this->dimensions = dimensions;
    maxSize = max(dimensions.x, max(dimensions.y, dimensions.z));
    stepSize = vec3(1.0f / (dimensions.x * (maxSize / dimensions.x)),
                    1.0f / (dimensions.y * (maxSize / dimensions.y)),
                    1.0f / (dimensions.z * (maxSize / dimensions.z)));
    setAspectratios(ratios);
    // create volume texture
    is16Bits ? readVolumeFromFile16(filepath) : readVolumeFromFile8(filepath);
    // histogram compute
    extractHistogram();
    // gradients
    generateGradients();
}

void RaycastVolume::drawFrontCubeFace() const
{
    if (!isDrawable) return;

    gl::enable(GL_CULL_FACE, true);
    gl::cullFace(GL_BACK);
    vertexArrayObject->bind();
    gl::drawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, static_cast<GLuint *>(nullptr));
    gl::disable(GL_CULL_FACE);
}

void RaycastVolume::drawBackCubeFace() const
{
    if (!isDrawable) return;

    gl::enable(GL_CULL_FACE, true);
    gl::cullFace(GL_FRONT);
    vertexArrayObject->bind();
    gl::drawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, static_cast<GLuint *>(nullptr));
    gl::disable(GL_CULL_FACE);
}

void RaycastVolume::drawVolume() const
{
    if (!isDrawable) return;

    // draw front face cube positions to render target
    {
        frontFbo->bindFramebuffer(GL_DRAW_FRAMEBUFFER);
        gl::clear();
        positionsShader->bind();
        positionsShader->uniform("scaleFactor", scaleFactor);
        gl::setDefaultShaderVars();
        drawFrontCubeFace();
        frontFbo->unbindFramebuffer();
    }
    // draw back face cube positions to render target
    {
        backFbo->bindFramebuffer(GL_DRAW_FRAMEBUFFER);
        gl::clear();
        drawBackCubeFace();
        backFbo->unbindFramebuffer();
    }
    // raycast volume
    {
        gl::clear();
        raycastShader->bind();
        // scale volume
        gl::setDefaultShaderVars();
        // bind  textures
        frontFbo->getTexture2d(GL_COLOR_ATTACHMENT0)->bind(0);
        backFbo->getTexture2d(GL_COLOR_ATTACHMENT0)->bind(1);
        volumeTexture->bind(2);
        gradientTexture->bind(3);
        transferFunction->get1DTexture()->bind(4);
        noiseTexture->bind(5);
        // raycast parameters
        raycastShader->uniform("threshold", transferFunction->getThreshold());
        raycastShader->uniform("scaleFactor", scaleFactor);
        raycastShader->uniform("stepSize", stepSize * stepScale);
        raycastShader->uniform("iterations", static_cast<int>(maxSize * (1.0f / stepScale) * 2.0f));
        // lighting
        raycastShader->uniform("diffuseShading", enableDiffuseShading);
        raycastShader->uniform("light.direction", light.direction);
        raycastShader->uniform("light.ambient", light.ambient);
        raycastShader->uniform("light.diffuse", light.diffuse);
        // draw cube
        gl::ScopedBlend blend(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        vertexArrayObject->bind();
        gl::drawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, static_cast<GLuint *>(nullptr));
    }
}

void RaycastVolume::extractHistogram()
{
    std::array<uint32_t, 256> histogramData = {0};
    // create shader storage buffer
    histogramSsbo = gl::Ssbo::create(256 * sizeof(uint32_t), histogramData.data(), GL_DYNAMIC_COPY);
    histogramSsbo->bindBase(1);

    // compute histogram
    {
        // bind histogram compute shader
        histogramCompute->bind();
        // volume texture
        glBindImageTexture(0, volumeTexture->getId(), 0, true, 0, GL_READ_ONLY, GL_R8UI);
        bindBufferBase(histogramSsbo->getTarget(), 1, histogramSsbo);
        // compute histogram
        gl::dispatchCompute(1, 1, 1);
        // block to ensure completion
        gl::memoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    // extract histogram values
    uint32_t* ssboValues = reinterpret_cast<uint32_t*>(histogramSsbo->map(GL_WRITE_ONLY));
    memcpy(histogramData.data(), ssboValues, 256 * sizeof(uint32_t));
    histogramSsbo->unmap();
    // convert to normalized floating values
    auto maxValue = max_element(histogramData.begin(), histogramData.end());

    // normalize and insert values
    for (int i = 0; i < 256; i++)
    {
        histogram[i] = static_cast<float>(histogramData[i]) / *maxValue;
    }
}

void RaycastVolume::generateGradients()
{
    auto format = gl::Texture3d::Format().magFilter(GL_LINEAR)
                                         .minFilter(GL_LINEAR)
                                         .wrapS(GL_CLAMP_TO_BORDER)
                                         .wrapR(GL_CLAMP_TO_BORDER)
                                         .wrapT(GL_CLAMP_TO_BORDER)
                                         .internalFormat(GL_RG16F);
    format.setDataType(GL_FLOAT);
    gradientTexture = gl::Texture3d::create(dimensions.x, dimensions.y, dimensions.z, format);

    // compute gradients
    {
        gradientsCompute->bind();
        // pass textures
        glBindImageTexture(0, volumeTexture->getId(), 0, true, 0, GL_READ_ONLY, GL_R8UI);
        glBindImageTexture(1, gradientTexture->getId(), 0, true, 0, GL_WRITE_ONLY, GL_RG16F);
        // compute gradients
        gl::dispatchCompute(ceil(dimensions.x / 8), ceil(dimensions.y / 8), ceil(dimensions.z / 8));
    }
    // smooth gradients
    {
        smoothGradientsCompute->bind();
        // pass textures
        glBindImageTexture(0, gradientTexture->getId(), 0, true, 0, GL_READ_WRITE, GL_RG16F);
        // compute gradients
        gl::dispatchCompute(ceil(dimensions.x / 8), ceil(dimensions.y / 8), ceil(dimensions.z / 8));
    }
}

const std::array<float, 256> &RaycastVolume::getHistogram() const
{
    return histogram;
}

void RaycastVolume::setTransferFunction(const std::shared_ptr<TransferFunction>& transferFunction)
{
    this->transferFunction = transferFunction;
}

void RaycastVolume::diffuseShading(bool enable)
{
    this->enableDiffuseShading = enable;
}

void RaycastVolume::setLight(vec3 direction, vec3 ambient, vec3 diffuse)
{
    light.direction = direction;
    light.ambient = ambient;
    light.diffuse = diffuse;
}

const Light& RaycastVolume::getLight() const
{
    return light;
}
