/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Mesh resource declarations.
 */

#pragma once

#include "../Core.hpp"
#include "Resource.hpp"
#include "nfCommon/Math/Box.hpp"
#include "nfCommon/Memory/Aligned.hpp"
#include "nfResources/MeshFile.hpp"

#include "../nfRenderer/RendererResources.hpp"


// TODO generating mesh at the runtime

namespace NFE {
namespace Resource {

/**
 * Part of the Mesh.
 * @details Submesh is meant to be using one material
 */
struct NFE_ALIGN(16) SubMesh : public Common::Aligned<16>
{
    Math::Box localBox;
    Material* material;
    uint32 indexOffset;
    uint32 trianglesCount;
};

using SubMeshes = std::vector<SubMesh>;

/**
 * Mesh resource class.
 */
class CORE_API NFE_ALIGN(16) Mesh : public ResourceBase
{
private:
    SubMeshes mSubMeshes;
    Math::Box mLocalBox;

    Renderer::BufferPtr mVB;
    Renderer::BufferPtr mIB;

    void Release();

public:
    Mesh();
    virtual ~Mesh();

    bool OnLoad();
    void OnUnload();

    NFE_INLINE const Renderer::BufferPtr& GetVertexBuffer() const { return mVB; }
    NFE_INLINE const Renderer::BufferPtr& GetIndexBuffer() const { return mIB; }
    NFE_INLINE const SubMeshes& GetSubMeshes() const { return mSubMeshes; }
    NFE_INLINE const Math::Box& GetLocalBox() const { return mLocalBox; }
};

} // namespace Resource
} // namespace NFE
