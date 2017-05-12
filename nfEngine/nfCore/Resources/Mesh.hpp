/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Mesh resource declarations.
 */

#pragma once

#include "../Core.hpp"
#include "Resource.hpp"
#include "../Renderer/RendererResources.hpp"
#include "nfCommon/Math/Box.hpp"
#include "nfCommon/Memory/Aligned.hpp"
#include "nfResources/MeshFile.hpp"


// TODO generating mesh at the runtime

namespace NFE {
namespace Resource {

/**
 * Part of the Mesh.
 * @details Submesh is meant to be using one material
 */
NFE_ALIGN(16)
struct SubMesh : public Common::Aligned<16>
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
NFE_ALIGN(16)
class CORE_API Mesh : public ResourceBase
{
    NFE_DECLARE_POLYMORPHIC_CLASS(Mesh)

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

    /**
     * Calculate approximate transformed mesh AABB.
     * @param matrix Transformation matrix applied to the mesh vertices.
     */
    Math::Box GetGlobalAABB(const Math::Matrix& matrix) const;
};

} // namespace Resource
} // namespace NFE
