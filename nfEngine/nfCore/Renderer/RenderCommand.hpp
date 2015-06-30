/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of RenderCommand class.
 */

#pragma once

#include "../Core.hpp"
#include "../Aligned.hpp"
#include "../Renderers/RendererInterface/Buffer.hpp"

namespace NFE {
namespace Renderer {

NFE_ALIGN16
struct InstanceData
{
    Math::Vector worldMatrix[3];
    Math::Vector velocity; //additional data
    Math::Vector angularVelocity; //additional data
};

NFE_ALIGN16
struct RenderCommand
{
    //instance data
    Math::Matrix matrix;
    Math::Vector velocity;
    Math::Vector angularVelocity;

    float distance;

    //mesh data
    IBuffer* pIB;
    IBuffer* pVB;
    const RendererMaterial* material;
    uint32 startIndex;
    uint32 indexCount;
};

class RenderCommandBuffer
{
public:
    // use custom aligned allocator because RenderCommand uses SIMD types
    std::vector<RenderCommand, Util::AlignedAllocator<RenderCommand, 64>> commands;

    void PushBack(const RenderCommand& command);
    void Sort();
    void Clear();
};

} // namespace Renderer
} // namespace NFE
