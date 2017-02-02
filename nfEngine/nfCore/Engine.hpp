/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Basic engine's API declarations.
 */

#pragma once

#include "Core.hpp"
#include "Resources/ResourcesManager.hpp"
#include "Scene/SceneManager.hpp"
#include "Renderer/HighLevelRenderer.hpp"
#include "nfCommon/Utils/ThreadPool.hpp"
#include "nfCommon/Containers/DynArray.hpp"
#include "nfCommon/Containers/UniquePtr.hpp"
#include "nfCommon/Containers/String.hpp"

#include <mutex> // TODO remove

namespace NFE {

struct UpdateRequest
{
    Scene::SceneManager* scene;
    float deltaTime;
};


class CORE_API Engine final
{
    NFE_MAKE_NONCOPYABLE(Engine)
    NFE_MAKE_NONMOVEABLE(Engine)

private:
    /**
     * Mutex used to synchronize scene rendering and resources allocations.
     * Resources must be released outside rendering stage.
     */
    std::recursive_mutex mRenderingMutex; // TODO get rid of that

    Common::ThreadPool mMainThreadPool;
    Resource::ResManager mResManager;
    Common::UniquePtr<Renderer::HighLevelRenderer> mRenderer;
    Common::DynArray<Scene::SceneManagerPtr> mScenes;

    bool OnInit();
    void OnRelease();
    Engine();

public:
    /**
     * Acquire pointer to the Engine instance.
     * If the function was not called before, the engine will be initialized.
     *
     * @return Valid Engine class instance or NULL on failure. See logs for more information.
     */
    static Engine* GetInstance();

    /**
     * Release engine.
     * @note This function must be called on an application shutdown.
     */
    static void Release();

    /**
     * Get engine's resources manager.
     *
     * @return NULL on failure.
     */
    NFE_INLINE Resource::ResManager* GetResManager()
    {
        return &mResManager;
    }

    /**
     * Get engine's high-level renderer.
     *
     * @return NULL on failure.
     */
    NFE_INLINE Renderer::HighLevelRenderer* GetRenderer() const
    {
        return mRenderer.Get();
    }

    /**
     * Get engine's main threadpool.
     *
     * @note The threadpool is used by the engine subsystems.
     * @return NULL on failure.
     */
    NFE_INLINE Common::ThreadPool* GetThreadPool()
    {
        return &mMainThreadPool;
    }

    /**
     * Create a new scene.
     *
     * @return NULL on failure.
     */
    Scene::SceneManager* CreateScene(const Common::String& name = "unnamed_scene");

    /**
     * Destroy a scene and all its systems.
     * @return  True if the scene was found and destroyed.
     */
    bool DeleteScene(Scene::SceneManager* scene);

    /**
     * Update physics and/or draw a scene(s).
     *
     * @param  views             List of scene draw requests.
     * @param  updateRequests    List of scene update requests.
     * @return True on success.
     */
    bool Advance(const Common::ArrayView<Renderer::View*> views, const Common::ArrayView<const UpdateRequest> updateRequests);

    NFE_INLINE std::recursive_mutex& GetRenderingMutex()
    {
        return mRenderingMutex;
    }
};

} // namespace NFE
