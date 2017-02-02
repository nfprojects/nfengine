/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of renderer system.
 */

#pragma once

#include "../../Core.hpp"
#include "../../Renderer/View.hpp"
#include "System.hpp"

#include "nfCommon/Utils/ThreadPool.hpp"
#include "nfCommon/Memory/Aligned.hpp"


namespace NFE {
namespace Scene {


/**
 * Scene system - high level renderer (interface).
 */
class RendererSystem : public ISystem
{
    NFE_DECLARE_POLYMORPHIC_CLASS(RendererSystem)
    NFE_MAKE_NONCOPYABLE(RendererSystem)

public:
    static const int ID = 3;

    explicit RendererSystem(SceneManager* scene);

    /**
     * Get rendering scene.
     */
    Renderer::RenderScene* GetRenderScene() const { return mRenderScene.Get(); }

    void Update(float dt) override;

private:
    Common::UniquePtr<Renderer::RenderScene> mRenderScene;
};

} // namespace Scene
} // namespace NFE
