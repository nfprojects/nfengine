/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of renderer system.
 */

#pragma once

#include "../Core.hpp"

#include "System.hpp"

#include "../Renderer/RendererResources.hpp"
#include "../Renderers/RendererInterface/CommandBuffer.hpp"

#include "nfCommon/Aligned.hpp"
#include "nfCommon/PackedArray.hpp"
#include "nfCommon/ThreadPool.hpp"
#include "nfCommon/Math/Frustum.hpp"

namespace NFE {
namespace Scene {

using RenderProxyID = uint32;

struct NFE_ALIGN16 MeshProxy
{
    Math::Matrix transform;
    Math::Vector velocity;
    Math::Vector angularVelocity;

    // TODO: place IB and VB here instead?
    Resource::Mesh* mesh;

    NFE_INLINE MeshProxy()
        : mesh(nullptr)
    { }
};

struct NFE_ALIGN16 LightProxy
{
    Math::Matrix transform;
    Math::Frustum frustum;
    Math::Matrix viewMatrix;
    Math::Matrix projMatrix;

    std::atomic_bool shadowDrawn;

    // required because of atomic member
    NFE_INLINE LightProxy& operator= (const LightProxy& other)
    {
        frustum = other.frustum;
        viewMatrix = other.viewMatrix;
        projMatrix = other.projMatrix;
        transform = other.transform;
        shadowDrawn = false;
        return *this;
    }
};

static const RenderProxyID INVALID_RENDER_PROXY;

} // namespace Scene
} // namespace NFE
