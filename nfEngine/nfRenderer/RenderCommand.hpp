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
#include "nfCommon/Math/Vector4.hpp"
#include "nfCommon/Math/Matrix4.hpp"


namespace NFE {
namespace Renderer {

struct NFE_ALIGN(16) InstanceData
{
    Math::Vector4 worldMatrix[3];
    Math::Vector4 velocity;
    Math::Vector4 angularVelocity;
};

struct NFE_ALIGN(16) RenderCommand
{
    //instance data
    Math::Matrix4 matrix;
    Math::Vector4 velocity;
    Math::Vector4 angularVelocity;

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
