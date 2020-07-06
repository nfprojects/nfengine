#pragma once

#include "../RendererApi.hpp"
#include "../../Renderers/RendererCommon/Types.hpp"

namespace NFE {
namespace Renderer {

class NFE_RENDERER_API IViewport
{
public:
    virtual ~IViewport();

    virtual void Resize(uint32 width, uint32 height) = 0;

    virtual uint32 GetWidth() const = 0;
    virtual uint32 GetHeight() const = 0;

    virtual BackbufferPtr GetBackbuffer() const = 0;

    virtual void Present() = 0;
};

} // namespace Renderer
} // namespace NFE
