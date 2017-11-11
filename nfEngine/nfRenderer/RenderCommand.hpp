/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of RenderCommand class.
 */

#pragma once

#include "nfRenderer.hpp"

#include "../Renderers/RendererInterface/Buffer.hpp"

#include "nfCommon/Containers/DynArray.hpp"
#include "nfCommon/Memory/Aligned.hpp"
#include "nfCommon/Math/Vector.hpp"
#include "nfCommon/Math/Matrix.hpp"


namespace NFE {
namespace Renderer {

struct NFE_ALIGN(16) InstanceData
{
    Math::Vector worldMatrix[3];
    Math::Vector velocity;
    Math::Vector angularVelocity;
};

struct NFE_ALIGN(16) RenderCommand
{
    //instance data
    Math::Matrix matrix;
    Math::Vector velocity;
    Math::Vector angularVelocity;

    float distance;

    //mesh data
    BufferPtr indexBuffer;
    BufferPtr vertexBuffer;
    const RendererMaterial* material;
    uint32 startIndex;
    uint32 indexCount;
};

class RenderCommandBuffer
{
public:
    // use custom aligned allocator because RenderCommand uses SIMD types
    Common::DynArray<RenderCommand> commands;

    void PushBack(const RenderCommand& command);
    void Sort();
    void Clear();
};

} // namespace Renderer
} // namespace NFE
