/**
 * @file   MeshComponent.hpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of mesh component class.
 */

#pragma once
#include "Core.hpp"
#include "Mesh.hpp"
#include "Component.hpp"

namespace NFE {
namespace Scene {

// Mesh entity descriptor used for serialization.
#pragma pack(push, 1)
struct MeshComponentDesc
{
    char meshResourceName[RES_NAME_MAX_LENGTH];
};
#pragma pack(pop)


class CORE_API MeshComponent : public Component
{
    friend class SceneManager;

private:
    Resource::Mesh* mMesh;
    Math::Box mGlobalAABB;

    void OnRenderDebug(Render::IRenderContext* pCtx);
    void CalcAABB();

public:
    MeshComponent(Entity* pParent);
    ~MeshComponent();

    /**
     * Set mesh resource by pointer
     */
    Result SetMeshResource(Resource::Mesh* pResource);

    /**
     * Set mesh resource by name
     */
    Result SetMeshResource(const char* pName);

    Result Deserialize(Common::InputStream* pStream);
    Result Serialize(Common::OutputStream* pStream) const;
};

} // namespace Scene
} // namespace NFE
