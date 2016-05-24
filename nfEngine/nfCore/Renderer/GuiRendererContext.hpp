/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of GUI Renderer's context
 */

#pragma once

#include "nfCommon/Math/Math.hpp"

namespace NFE {
namespace Renderer {

using namespace Math;

struct GuiQuadData
{
    IResourceBindingInstance* textureBinding;
    bool alphaTexture;

    GuiQuadData()
        : textureBinding(nullptr)
        , alphaTexture(false)
    {}

    GuiQuadData(IResourceBindingInstance* textureBinding, bool alphaTexture)
        : textureBinding(textureBinding)
        , alphaTexture(alphaTexture)
    {}
};

struct GuiQuadVertex
{
    Rectf rect;
    Rectf texCoord;
    uint32 color;

    GuiQuadVertex()
        : rect(Rectf())
        , texCoord(Rectf())
        , color(0xFFFFFFFF)
    {}

    GuiQuadVertex(const Rectf& rect, const Rectf& texCoord, uint32 color)
        : rect(rect)
        , texCoord(texCoord)
        , color(color)
    {}
};

/**
 * GUI renderer's per context data
 */
struct GuiRendererContext
{
    size_t queuedQuads;
    int vertexBufferSize;
    int indexBufferSize;

    // texture data
    std::unique_ptr<GuiQuadData[]> quadData;

    // geometry and color data (pushed directly to vertex buffer)
    std::unique_ptr<GuiQuadVertex[]> quadVertices;

    GuiRendererContext();
};

} // namespace Renderer
} // namespace NFE
