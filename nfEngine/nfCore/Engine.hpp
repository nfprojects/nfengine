/**
 * @file   Engine.hpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Basic engine's API declarations.
 */

#pragma once

#include "Core.hpp"
#include "ResourcesManager.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Texture.hpp"
#include "SceneManager.hpp"
#include "../nfCommon/Window.hpp"
#include "../nfCommon/ThreadPool.hpp"

namespace NFE {

struct DrawRequest
{
    Render::View* pView;

    // used for auto exposure
    float deltaTime;
};

struct UpdateRequest
{
    Scene::SceneManager* pScene;
    float deltaTime;
};

/**
 *  Initialize engine. This function must be called on an application startup.
 * @return Result::OK on success. See logs for more information.
 */
CORE_API Result EngineInit();

/**
 *  Initialize engine. This function must be called on an application shutdown.
 * @return Result::OK on success. See logs for more information.
 */
CORE_API Result EngineRelease();

/**
 *  Get engine's resources manager.
 * @return NULL on failure.
 */
CORE_API Resource::ResManager* EngineGetResManager();

/**
 *  Get engine's resources renderer.
 * @return NULL on failure.
 */
CORE_API Render::IRenderer* EngineGetRenderer();

/**
 *  Create a new scene.
 * @return NULL on failure.
 */
CORE_API Scene::SceneManager* EngineCreateScene();

/**
 *  Destroy a scene and all its entities.
 * @return Result::OK on success.
 */
CORE_API Result EngineDeleteScene(Scene::SceneManager* pScene);

/**
 *  Update physics and/or draw a scene(s).
 * @param  pDrawRequests List of scene draw requests.
 * @param  pUpdateRequests List of scene updage requests.
 * @return Result::OK on success.
 */
CORE_API Result EngineAdvance(const DrawRequest* pDrawRequests, uint32 drawRequestsNum,
                              const UpdateRequest* pUpdateRequests, uint32 updateRequestsNum);

/**
 *  Get resource pointer by name. If the resource with given name does not exist, it is
 *         created automatically.
 * @param  resType Resource type.
 * @param  pResName Resource name. Maximum resource name is restricted by RES_NAME_MAX_LENGTH
 * @param  check Do not create a resource if it does not exist.
 * @return NULL on failure or valid resource pointer.
 */
CORE_API Resource::ResourceBase* EngineGetResource(Resource::ResourceType resType,
        const char* pResName, bool check = false);


#define ENGINE_GET_TEXTURE(name) \
    ((Texture*)EngineGetResource(ResourceType::Texture, name))

#define ENGINE_GET_MATERIAL(name) \
    ((Material*)EngineGetResource(ResourceType::Material, name))

#define ENGINE_GET_MESH(name) \
    ((Mesh*)EngineGetResource(ResourceType::Mesh, name))

#define ENGINE_GET_COLLISION_SHAPE(name) \
    ((CollisionShape*)EngineGetResource(ResourceType::CollisionShape, name))

/**
 *  Add custom resource object to the engine's resources list.
 * @param  pResource Pointer to a user's resource object.
 * @param  pName Name of the resource. If NULL, the name is obtained from the object.
 * @return Result::OK on success.
 */
CORE_API Result EngineAddResource(Resource::ResourceBase* pResource, const char* pName = 0);


} // namespace NFE
