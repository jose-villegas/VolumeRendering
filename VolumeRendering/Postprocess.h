#pragma once
#include "cinder/gl/gl.h"

class PostProcess
{
public:
    /**
     * \brief Displays the given texture on a full screen quad. If colorTex is 
     * null then it displays the color output of the last post-process effect
     * \param colorTex The texture to display
     */
    void displayTexture(const ci::gl::Texture2dRef colorTex = nullptr) const;
    /**
     * \brief Reinhard tonemapping and gamma correction
     * \param hdrBuffer An unclampled (floating point) color buffer
     */
    void toneMapping(const cinder::gl::Texture2dRef& hdrBuffer) const;
    /**
     * \brief The temporal color texture contains the output color result of
     * the last post-process effect executed
     * \return The temporal color texture
     */
    const ci::gl::Texture2dRef &getTemporalColorTexture() const;
    /**
     * \brief Fast approximate anti-aliasing post effect
     * \param texture The color buffer texture
     */
    void fxAA(const cinder::gl::Texture2dRef &texture = nullptr) const;
    /**
     * \brief Resizes the created framebuffers to the actual window's size
     */
    void resizeFbos();
    PostProcess();
    ~PostProcess();
private:
    // fs screen effects
    ci::gl::BatchRef textureRect;
    ci::gl::BatchRef toneMappingRect;
    ci::gl::BatchRef fxaaRect;

    // temporal texture
    ci::gl::Texture2dRef temporalTexture;
    ci::gl::FboRef temporalFbo;
};

