/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Basic engine's API declarations.
 */

#pragma once

#include "Core.hpp"
#include "ResourcesManager.hpp"
#include "SceneManager.hpp"
#include "Renderer/HighLevelRenderer.hpp"
#include "../nfCommon/ThreadPool.hpp"

namespace NFE {

struct DrawRequest
{
    Renderer::View* view;
};

struct UpdateRequest
{
    Scene::SceneManager* scene;
    float deltaTime;
};


class CORE_API Engine
{
    friend CORE_API Engine* EngineInit();
    friend CORE_API bool EngineRelease();

    /**
     * Mutex used to synchronize scene rendering and resources allocations.
     * Resources must be released outside rendering stage.
     */
    std::mutex mRenderingMutex;

    Common::ThreadPool mMainThreadPool;
    Resource::ResManager mResManager;
    std::unique_ptr<Renderer::HighLevelRenderer> mRenderer;
    std::set<Scene::SceneManager*> mScenes;

    bool OnInit();
    void OnRelease();
    Engine();

    /// disable all other constructors and assignment operators
    Engine(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine& operator=(Engine&&) = delete;

public:
    /**
     * @return Valid Engine class instance or NULL on failure. See logs for more information.
     */
    static Engine* Initialize();

    /**
     * Aquire pointer to the Engine instance.
     *
     * @return Valid Engine class instance or NULL on failure. See logs for more information.
     */
    static Engine* GetInstance();

    /**
     * Release engine. This function must be called on an application shutdown.
     *
     * @return True on success. See logs for more information.
     */
    static bool Release();

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
     * @return True on success.
     */
    bool DeleteScene(Scene::SceneManager* scene);

    /**
     * Update physics and/or draw a scene(s).
     *
     * @param  drawRequests List of scene draw requests.
     * @param  dpdateRequests List of scene updage requests.
     * @return True on success.
     */
    bool Advance(const DrawRequest* drawRequests, uint32 drawRequestsNum,
                 const UpdateRequest* updateRequests, uint32 updateRequestsNum);


    NFE_INLINE std::mutex& GetRenderingMutex()
    {
        return mRenderingMutex;
    }
};

} // namespace NFE
