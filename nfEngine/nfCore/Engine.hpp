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
    std::recursive_mutex mRenderingMutex;

    Common::ThreadPool mMainThreadPool;
    Resource::ResManager mResManager;
    std::unique_ptr<Renderer::HighLevelRenderer> mRenderer;
    std::set<Scene::SceneManager*> mScenes;

    bool OnInit();
    void OnRelease();
    Engine();

public:
    /**
     * Aquire pointer to the Engine instance.
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
        return mRenderer.get();
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
    Scene::SceneManager* CreateScene();

    /**
     * Destroy a scene and all its entities.
     */
    void DeleteScene(Scene::SceneManager* scene);

    /**
     * Update physics and/or draw a scene(s).
     *
     * @param  views             List of scene draw requests.
     * @param  viewsNum          Number of elements in @p views array.
     * @param  updateRequests    List of scene updage requests.
     * @param  updateRequestsNum Number of elements in @p updateRequests array.
     * @return True on success.
     */
    bool Advance(Renderer::View** views, size_t viewsNum,
                 const UpdateRequest* updateRequests, size_t updateRequestsNum);


    NFE_INLINE std::recursive_mutex& GetRenderingMutex()
    {
        return mRenderingMutex;
    }
};

} // namespace NFE
