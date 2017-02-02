/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of mesh component class.
 */

#pragma once

#include "Component.hpp"
#include "../../Resources/Mesh.hpp"
#include "../../Scene/Systems/RenderProxies.hpp"

#include "nfCommon/Memory/Aligned.hpp"


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

    RenderProxyID mRenderingProxy;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate() override;

    void DeleteRenderingProxy();
    void CreateRenderingProxy();

    void OnMeshResourceLoaded();
};

} // namespace Scene
} // namespace NFE
