#include "PostProcess.h"
#include <cinder/app/AppBase.h>
#include <cinder/Log.h>
#include "RenderingParams.h"
using namespace ci;
using namespace glm;
using namespace app;

void PostProcess::displayTexture(const gl::Texture2dRef colorTex) const
{
    const gl::ScopedViewport scopedViewport(ivec2(0), toPixels(getWindowSize()));
    const gl::ScopedMatrices scopedMatrices;
    const gl::ScopedTextureBind scopedTextureBind(colorTex ? colorTex : temporalTexture, 0);

    // no depth write or read for quad display
    gl::disableDepthRead();
    gl::disableDepthWrite();
    gl::clear();

    // translate to center and scale to fit whole screen
    gl::translate(getWindowCenter());
    gl::scale(getWindowSize());

    // draw quad with mapped texture
    textureRect->draw();
}

void PostProcess::toneMapping(const gl::Texture2dRef& hdrBuffer) const
{
    const gl::ScopedFramebuffer scopedFramebuffer(temporalFbo);
    const gl::ScopedViewport scopedViewport(ivec2(0), temporalFbo->getSize());
    const gl::ScopedMatrices scopedMatrices;
    const gl::ScopedTextureBind scopedTextureBind(hdrBuffer, 0);

    // custom uniforms
    auto& prog = toneMappingRect->getGlslProg();
    prog->uniform("gamma", RenderingParams::GetGamma());
    prog->uniform("exposure", RenderingParams::GetExposure());

    // no depth write or read for quad display
    gl::disableDepthRead();
    gl::disableDepthWrite();
    gl::clear();

    // translate to center and scale to fit whole screen
    gl::translate(getWindowCenter());
    gl::scale(getWindowSize());

    // draw quad with mapped texture
    toneMappingRect->draw();
}

void PostProcess::fxAA(const gl::Texture2dRef& texture) const
{
    const gl::ScopedViewport scopedViewport(ivec2(0), toPixels(getWindowSize()));
    const gl::ScopedMatrices scopedMatrices;
    const gl::ScopedTextureBind scopedTextureBind(texture ? texture : temporalTexture, 0);

    // custom uniforms
    auto& prog = fxaaRect->getGlslProg();
    prog->uniform("pixelSize", vec2(1.0f) / vec2(toPixels(getWindowSize())));

    // no depth write or read for quad display
    gl::disableDepthRead();
    gl::disableDepthWrite();
    gl::clear();

    // translate to center and scale to fit whole screen
    gl::translate(getWindowCenter());
    gl::scale(getWindowSize());

    // draw quad with mapped texture
    fxaaRect->draw();
}

const gl::Texture2dRef &PostProcess::getTemporalColorTexture() const
{
    return temporalTexture;
}

void PostProcess::resizeFbos()
{
    // temporal accumulated texture
    gl::Texture2d::Format colorAccFormat = gl::Texture2d::Format().internalFormat(GL_RGB8)
                                                                  .magFilter(GL_NEAREST)
                                                                  .minFilter(GL_NEAREST)
                                                                  .wrap(GL_REPEAT);
    const ivec2 winSize = getWindowSize();
    const int32_t h = winSize.y;
    const int32_t w = winSize.x;

    try
    {
        // create temporal texture
        temporalTexture = gl::Texture2d::create(w, h, colorAccFormat);
        // deferred rendering mode gbuffer
        gl::Fbo::Format colorFormat;
        colorFormat.attachment(GL_COLOR_ATTACHMENT0, temporalTexture);
        colorFormat.depthBuffer();
        temporalFbo = gl::Fbo::create(w, h, colorFormat);
    }
    catch (const Exception& e)
    {
        CI_LOG_EXCEPTION("Fbo/Renderbuffer create", e);
    }
}

PostProcess::PostProcess()
{
    // create fbos
    resizeFbos();
    // post process programs 
    auto toneMappingProg = gl::GlslProg::create(gl::GlslProg::Format()
        .vertex(loadFile("shaders/fs_quad.vert"))
        .fragment(loadFile("shaders/tonemapping.frag")));
    auto fxaaProg = gl::GlslProg::create(gl::GlslProg::Format()
        .vertex(loadFile("shaders/fs_quad.vert"))
        .fragment(loadFile("shaders/fxaa.frag")));
    // create fs quad for single texture display
    const gl::GlslProgRef stockTexture = gl::context()->getStockShader(gl::ShaderDef().texture(GL_TEXTURE_2D));
    const gl::VboMeshRef rect = gl::VboMesh::create(geom::Rect());
    textureRect = gl::Batch::create(rect, stockTexture);
    toneMappingRect = gl::Batch::create(rect, toneMappingProg);
    fxaaRect = gl::Batch::create(rect, fxaaProg);
}

PostProcess::~PostProcess() {}
