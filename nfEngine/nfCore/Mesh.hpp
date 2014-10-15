/**
    NFEngine project

    \file   Mesh.hpp
    \brief  Mesh resource declarations.
*/

#pragma once

#include "Core.hpp"
#include "Resource.hpp"
#include "Renderer.hpp"

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
NFE_ALIGN(16)
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
NFE_ALIGN(16)
class CORE_API Mesh : public ResourceBase
{
    friend class Scene::MeshComponent;
    friend class Scene::SceneManager;

private:
    Render::MeshVertex* mVerticies;
    uint32* mIndicies;

    SubMesh* mSubMeshes;
    uint32 mVeriticesCount;
    uint32 mIndiciesCount;
    uint32 mSubMeshesCount;

    Math::Box mLocalBox;

    Render::IRendererBuffer* mVB;
    Render::IRendererBuffer* mIB;

protected:
    /// when creating custom mesh, use this functions:
    Result AllocateVerticies(uint32 count);
    Result AllocateIndicies(uint32 count);
    Result AllocateSubmeshes(uint32 count);

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
