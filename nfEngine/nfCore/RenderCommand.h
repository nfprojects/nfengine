/**
    NFEngine project

    \file   RenderCommand.cpp
    \brief  Declarations of RenderCommand class.
*/

#pragma once

#include "Core.h"
#include "Aligned.h"

namespace NFE {
namespace Render {

NFE_ALIGN(16)
struct InstanceData
{
    Math::Vector worldMatrix[3];
    Math::Vector velocity; //additional data
    Math::Vector angularVelocity; //additional data
};

NFE_ALIGN(16)
struct RenderCommand
{
    //instance data
    Math::Matrix matrix;
    Math::Vector velocity;
    Math::Vector angularVelocity;

    float distance;

    //mesh data
    IRendererBuffer* pIB;
    IRendererBuffer* pVB;
    const RendererMaterial* pMaterial;
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

} // namespace Render
} // namespace NFE
