/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of mesh component class.
 */

#pragma once

#include "../Core.hpp"
#include "../Resources/Mesh.hpp"
#include "Component.hpp"
#include "../nfCommon/Aligned.hpp"

namespace NFE {
namespace Scene {

// Mesh entity descriptor used for serialization.
#pragma pack(push, 1)
struct MeshComponentDesc
{
    char meshResourceName[RES_NAME_MAX_LENGTH];
};
#pragma pack(pop)

NFE_ALIGN16
class CORE_API MeshComponent
    : public ComponentBase<MeshComponent>
    , public Common::Aligned<16>
{
    friend class SceneManager;
    friend class RendererSystem;

    Resource::Mesh* mMesh;
    Math::Box mGlobalAABB;

    void CalcAABB(const Math::Matrix& transform);

public:
    MeshComponent();
    ~MeshComponent();

    /**
     * Set mesh resource by pointer.
     */
    bool SetMeshResource(Resource::Mesh* resource);

    /**
     * Set mesh resource by name.
     */
    bool SetMeshResource(const char* name);
};

} // namespace Scene
} // namespace NFE
