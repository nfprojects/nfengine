/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of GUI Renderer's context
 */

#pragma once

#include "nfCommon/Math/Math.hpp"
#include "nfCommon/Math/Rectangle.hpp"

namespace NFE {
namespace Renderer {

using namespace Math;

struct GuiQuadData
{
    ResourceBindingInstancePtr textureBinding;
    bool alphaTexture;

    GuiQuadData()
        : textureBinding(nullptr)
        , alphaTexture(false)
    {}

    GuiQuadData(ResourceBindingInstancePtr textureBinding, bool alphaTexture)
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
        : color(0xFFFFFFFF)
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
    static const size_t gQuadsBufferSize;

    CommandRecorderPtr commandRecorder;
    size_t queuedQuads;
    int vertexBufferSize;
    int indexBufferSize;

    // texture data
    std::unique_ptr<GuiQuadData[]> quadData;

    // geometry and color data (pushed directly to vertex buffer)
    std::unique_ptr<GuiQuadVertex[]> quadVertices;

    NFE_INLINE GuiRendererContext(const CommandRecorderPtr& commandRecorder)
        : commandRecorder(commandRecorder)
        , queuedQuads(0)
        , vertexBufferSize(0)
        , indexBufferSize(0)
    {
        quadData.reset(new GuiQuadData[gQuadsBufferSize]);
        quadVertices.reset(new GuiQuadVertex[gQuadsBufferSize]);
    }
};

} // namespace Renderer
} // namespace NFE
