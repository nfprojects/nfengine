/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of mesh component class.
 */

#pragma once

#include "../../Core.hpp"
#include "../../Resources/Mesh.hpp"
#include "Component.hpp"
#include "../../Scene/Systems/RenderProxies.hpp"
#include "nfCommon/Aligned.hpp"


namespace NFE {
namespace Scene {

NFE_ALIGN16
class CORE_API MeshComponent
    : public Component
    , public Common::Aligned<16>
{
public:
    MeshComponent();
    virtual ~MeshComponent();

    /**
     * Set mesh resource by pointer.
     */
    bool SetMeshResource(Resource::Mesh* resource);

    /**
     * Set mesh resource by name.
     */
    bool SetMeshResource(const char* name);

private:
    // TODO resource handle instead of manual ref counting
    Resource::Mesh* mMesh;

    // 
    RenderProxyID mRenderingProxy;

    virtual void OnDetach() override;

    void DeleteRenderingProxy();

    void OnMeshResourceLoaded();
};

} // namespace Scene
} // namespace NFE
