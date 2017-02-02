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

class CORE_API MeshComponent : public IComponent
{
public:
    MeshComponent();
    ~MeshComponent();

    // IComponent interface implementation
    virtual Math::Box GetBoundingBox() const override;

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

    virtual void OnAttach(Entity* entity) override;
    virtual void OnDetach() override;

    void DeleteRenderingProxy();
    void CreateRenderingProxy();

    void OnMeshResourceLoaded();
};

} // namespace Scene
} // namespace NFE
