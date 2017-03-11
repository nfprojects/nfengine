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
NFE_ALIGN16
struct SubMesh : public Common::Aligned<16>
{
    Math::Box localBox;
    Material* material;
    uint32 indexOffset;
    uint32 trianglesCount;
};

/**
 * Mesh resource class.
 */
NFE_ALIGN16
class CORE_API Mesh : public ResourceBase
{
    friend class Scene::RendererSystem;
    friend class Scene::MeshComponent;
    friend class Scene::SceneManager;
    friend class Renderer::DebugRenderer;

private:
    std::vector<SubMesh> mSubMeshes;
    Math::Box mLocalBox;

    Renderer::BufferPtr mVB;
    Renderer::BufferPtr mIB;

    void Release();

public:
    Mesh();
    virtual ~Mesh();

    bool OnLoad();
    void OnUnload();

    /**
     * Calculate approximate transformed mesh AABB.
     * @param matrix Transformation matrix aplied to the mesh vertices.
     */
    Math::Box GetGlobalAABB(const Math::Matrix& matrix) const;
};

} // namespace Resource
} // namespace NFE
