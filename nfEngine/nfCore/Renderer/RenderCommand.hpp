/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of RenderCommand class.
 */

#pragma once

#include "../Core.hpp"
#include "Renderers/RendererInterface/Buffer.hpp"

#include "nfCommon/Memory/Aligned.hpp"
#include "nfCommon/Math/Vector4.hpp"
#include "nfCommon/Math/Matrix4.hpp"

#include <vector>


namespace NFE {
namespace Renderer {

struct InstanceData
{
    Math::Vector4 worldMatrix[3];
    Math::Vector4 velocity;
    Math::Vector4 angularVelocity;
};

struct RenderCommand
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
    std::vector<RenderCommand, Common::AlignedAllocator<RenderCommand, 64>> commands;

    void PushBack(const RenderCommand& command);
    void Sort();
    void Clear();
};

} // namespace Renderer
} // namespace NFE
