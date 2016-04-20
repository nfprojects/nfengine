/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of OpenGL 4 Render Target.
 */

#pragma once

#include "../RendererInterface/RenderTarget.hpp"
#include "Defines.hpp"


namespace NFE {
namespace Renderer {

class RenderTarget : public IRenderTarget
{
    friend class CommandBuffer;

    GLuint mFBO;
    int mWidth;
    int mHeight;
    std::vector<GLenum> mAttachments;

public:
    RenderTarget();
    ~RenderTarget();

    void GetDimensions(int& width, int& height);
    bool Init(const RenderTargetDesc& desc);
};

} // namespace Renderer
} // namepsace NFE
