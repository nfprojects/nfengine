/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of OpenGL 4 Render Target.
 */

#pragma once

#include "../RendererInterface/RenderTarget.hpp"

namespace NFE {
namespace Renderer {

class RenderTarget : public IRenderTarget
{
private:

public:
    RenderTarget();
    ~RenderTarget();

    bool Init(const RenderTargetDesc& desc);
};

} // namespace Renderer
} // namepsace NFE
