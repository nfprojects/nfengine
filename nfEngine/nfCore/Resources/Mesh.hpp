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

namespace NFE {
namespace Resource {

struct CustomMeshVertex
{
    Math::Float3 position;
    Math::Float3 normal;
    Math::Float3 tangent;
    Math::Float2 texCoord;
};

struct CustomMeshSubMesh
{
    Material* material;
    uint32 indexOffset; // first index of the submesh
    uint32 trianglesCount; // how many triangles in the submesh
};

/**
 * Structure used only for reading mesh from a file.
 */
#define MAT_NAME_MAX_LENGTH (120)
struct SubMeshDesc
{
    uint32 indexOffset;
    uint32 triangleCount;
    char materialName[MAT_NAME_MAX_LENGTH];
};

/**
 * Part of the Mesh.
 * @details Submesh is meant to be using one material
 */
NFE_ALIGN16
struct SubMesh
{
    Math::Box localBox;
    Material* material;
    uint32 indexOffset;
    uint32 trianglesCount;
};

/**
 * Mesh resource class.
 * @details Can be derived to create custom mesh (not loaded from file, but created by an application).
 */
NFE_ALIGN16
class CORE_API Mesh : public ResourceBase
{
    friend class Scene::RendererSystem;
    friend class Scene::MeshComponent;
    friend class Scene::SceneManager;
    friend class Renderer::DebugRenderer;

private:
    Renderer::MeshVertex* mVerticies;
    uint32* mIndices;

    SubMesh* mSubMeshes;
    uint32 mVeriticesCount;
    uint32 mIndicesCount;
    uint32 mSubMeshesCount;

    Math::Box mLocalBox;

    std::unique_ptr<Renderer::IBuffer> mVB;
    std::unique_ptr<Renderer::IBuffer> mIB;

protected:
    /// when creating custom mesh, use this functions:
    bool AllocateVerticies(uint32 count);
    bool AllocateIndices(uint32 count);
    bool AllocateSubmeshes(uint32 count);

public:
    Mesh();
    virtual ~Mesh();

    bool OnLoad();
    void OnUnload();

    void Release();

    /**
     * Calculate approximate transformed mesh AABB.
     * @param matrix Transofmation matrix aplied to the mesh vertices.
     */
    Math::Box GetGlobalAABB(const Math::Matrix& matrix);
};

} // namespace Resource
} // namespace NFE
