/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of mesh component class.
 */

#pragma once

#include "../Core.hpp"
#include "../Resources/Mesh.hpp"
#include "Component.hpp"
#include "nfCommon/Aligned.hpp"

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
    // mesh resource
    Resource::Mesh* mMesh;

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

    /**
     * Calculate global-space AABB.
     */
    Math::Box CalculateAABB(const Math::Matrix& transform) const;

    NFE_INLINE Resource::Mesh* GetMeshResource() const
    {
        return mMesh;
    }
};

} // namespace Scene
} // namespace NFE
