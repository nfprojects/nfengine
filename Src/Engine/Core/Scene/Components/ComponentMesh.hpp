/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of mesh component class.
 */

#pragma once

#include "Component.hpp"

#include "../../../Common/Memory/Aligned.hpp"


namespace NFE {
namespace Scene {

/**
 * A component spawning mesh proxy on a render scene.
 */
class CORE_API MeshComponent : public IComponent
{
    NFE_DECLARE_POLYMORPHIC_CLASS(MeshComponent)

public:
    MeshComponent();
    ~MeshComponent();

    // IComponent interface implementation
    virtual Math::Box GetBoundingBox() const override;

    //bool SetMeshResource(Common::SharedPtr<Resource::Mesh> resource);


private:
    //Common::SharedPtr<Resource::Mesh> mMesh;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate() override;

    void DeleteRenderingProxy();
    void CreateRenderingProxy();

    void OnMeshResourceLoaded();

    RendererSystem* GetRendererSystem() const;
};

} // namespace Scene
} // namespace NFE
