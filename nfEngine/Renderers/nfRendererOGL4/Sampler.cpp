/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definition of OpenGL 4 renderer's sampler
 */

#include "PCH.hpp"
#include "Sampler.hpp"
#include "Translations.hpp"

namespace NFE {
namespace Renderer {

Sampler::~Sampler()
{
    glDeleteSamplers(1, &mSampler);
}

bool Sampler::Init(const SamplerDesc& desc)
{
    // generate sampler
    glGenSamplers(1, &mSampler);
    if (mSampler == GL_NONE)
    {
        NFE_LOG_ERROR("Failed to create Sampler object.");
        return false;
    }

    // set sampler up
    glSamplerParameteri(mSampler, GL_TEXTURE_MIN_FILTER, TranslateTextureMinFilter(desc.minFilter));
    glSamplerParameteri(mSampler, GL_TEXTURE_MAG_FILTER, TranslateTextureMagFilter(desc.magFilter));
    glSamplerParameteri(mSampler, GL_TEXTURE_WRAP_S, TranslateTextureWrapMode(desc.wrapModeU));
    glSamplerParameteri(mSampler, GL_TEXTURE_WRAP_T, TranslateTextureWrapMode(desc.wrapModeV));
    glSamplerParameteri(mSampler, GL_TEXTURE_WRAP_R, TranslateTextureWrapMode(desc.wrapModeW));
    glSamplerParameteri(mSampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, desc.maxAnisotropy);
    glSamplerParameterf(mSampler, GL_TEXTURE_MIN_LOD, desc.minMipmap);
    glSamplerParameterf(mSampler, GL_TEXTURE_MAX_LOD, desc.maxMipmap);
    glSamplerParameterf(mSampler, GL_TEXTURE_LOD_BIAS, desc.mipmapBias);
    glSamplerParameteri(mSampler, GL_TEXTURE_COMPARE_MODE, desc.compare ? GL_COMPARE_REF_TO_TEXTURE
                                                                        : GL_NONE);
    glSamplerParameteri(mSampler, GL_TEXTURE_COMPARE_FUNC, TranslateCompareFunc(desc.compareFunc));

    if (desc.borderColor)
        glSamplerParameterfv(mSampler, GL_TEXTURE_BORDER_COLOR, desc.borderColor);

    return true;
}

} // namespace Renderer
} // namespace NFE
