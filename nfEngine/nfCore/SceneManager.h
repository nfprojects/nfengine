/**
    NFEngine project

    \file   SceneManager.h
    \brief  Declaration of Scene class.
*/

#pragma once
#include "Core.h"
#include "Camera.h"
#include "MeshComponent.h"
#include "Light.h"
#include "Body.h"
#include "../nfCommon/ThreadPool.hpp"
#include "SceneEvent.h"

namespace NFE {
namespace Scene {

enum class SerializationFormat
{
    Binary,
    Xml
};

struct CompareEntityName
{
    bool operator()(const char* a, const char* b) const
    {
        return strcmp(a, b) < 0;
    }
};

NFE_ALIGNED_CLASS(CORE_API EnviromentDesc)
{
public:
    Math::Vector ambientLight;
    Math::Vector backgroundColor;

    EnviromentDesc()
    {
        ambientLight = Math::Vector(0.3f, 0.3f, 0.3f);
        backgroundColor = Math::Vector(0.3f, 0.3f, 0.3f);
    }
};

/**
 * Scene manager.
 */
NFE_ALIGNED_CLASS(CORE_API SceneManager)
{
    friend class Entity;
    friend class BodyComponent;
    friend class PhysicsWorld;
    friend class Segment;
    friend void PhysicsUpdateCallback(void* pUserData, int Instance, int ThreadID);

#ifdef NF_CORE_EXPORTS
private:
    std::vector<MeshComponent*> mMeshes;
    std::vector<LightComponent*> mLights;
    std::vector<BodyComponent*> mBodies;
    std::vector<Camera*> mCameras;

    Util::BVH* mMeshesBVH;

    // events queue
    EventSystem mEventSystem;

    //entities lists
    std::map<const char*, Entity*, CompareEntityName> mEntitiesMap;
    std::set<Entity*> mEntities;
    std::vector<Entity*> mEntitiesToRemove;
    Camera* mDefaultCamera;

    Segment* mFocusSegment;
    std::set<Segment*> mSegments;
    std::set<SegmentLink*> mLinks;
    //std::vector<Segment*> mLoadedSegments;

    // environment
    EnviromentDesc mEnvDesc;

    // physics
    float mDeltaTime;
    PhysicsWorld* mWorld;

    //entities with loaded meshes
    std::vector<MeshComponent*> mActiveMeshEntities;

    // Wait for physics update thread to finish.
    void SyncPhysics();

    /*
        Check if default camera escaped focus segment.
        If so, switch focus, unload/load segments and move coordinate system.
    */
    void UpdateSegments();
    void UpdateSegmentForEntity(Entity * pEntity);


    void DrawBVHNode(Render::IRenderContext * pCtx, uint32 node, uint32 depth);
#endif

public:
    SceneManager();
    ~SceneManager();

    /*
        Save scene (entities layout, etc.) to an output stream.
    */
    Result Serialize(Common::OutputStream * pStream, SerializationFormat format, bool patch);

    /*
        Load scene (entities layout, etc.) from an input file.
    */
    Result Deserialize(Common::InputStream * pStream, bool patch);


    /// Create a new scene segment.
    Segment* CreateSegment(const char* pName, const Math::Vector & size);

    /*
        Create a link between two segments.
        'offset' is displacement between 'pFrom' and 'pTo' segments' centers (order is important!).
    */
    Result CreateLink(Segment * pFrom, Segment * pTo, const Math::Vector & offset);

    // Get segment pointer by world position
    Segment* FindSegment(const Math::Vector & position);

    Result SetFocusSegment(Segment * pSegment);
    Segment* GetFocusSegment() const;

    /**
        Decode and create entity from input stream.
        Used, when a sector is loaded.
    **/
    Result DeserializeEntity(Common::InputStream * pStream, const Math::Vector & offset,
                             Entity** ppEntity);

    /*
        Create and add a new entity object the scene.
        pParent parameter is optional.
        When pName is null, entity name is generated automatically.
        Returns null, when error occurred (lack of memory usually).
    */
    Entity* CreateEntity(Entity* pParent = NULL, const char* pName = NULL);

    /**
     * Get entity pointer by its name
     * @return NULL on failure, valid pointer on success
     */
    Entity* GetEntityByName(const char* pName);

    /*
        Destroy (immediately) entity object and all its components.
        Warning: this function should be used wisely.
        It should be avoided to perform event callbacks,
        because of possibility of passing invalid entity pointer
        (destroy A, create B, destroy A). To solve this problem,
        use EnqueueDeleteEntity() method.
    */
    Result DeleteEntity(Entity * pEntity);

    /**
     * Enqueue entity deletion.
     * @details After exiting a call, pEntity is still valid - actual object removal is performed
     *          inside SceneManager::Update() method.
     * @param pEntity Entity to be removed.
     * @param recursive If true, remove whole child tree.
     * @return Result::OK on success.
     */
    Result EnqueueDeleteEntity(Entity * pEntity, bool recursive = true);


    void SetDefaultCamera(Camera * pCamera);
    Camera* GetDefaultCamera() const;

    // Environment
    void SetEnvironment(const EnviromentDesc * pDesc);
    void GetEnvironment(EnviromentDesc * pDesc) const;

    EventSystem* GetEventSystem();

    // Calculate physics, prepare scene for rendering.
    void Update(float deltaTime);

    // Perform Geometry Pass for pCamera in pContext
    void RenderGBuffer(Render::IRenderContext * pCtx, Camera * pCamera,
                       Render::CameraRenderDesc * pCameraDesc, Render::IRenderTarget * pRT);

    void RenderShadow(Render::IRenderContext * pCtx, LightComponent * pLight, uint32 faceID = 0);
    void Render(Camera * pCamera, Render::IRenderTarget * pRT);

    void FindActiveMeshEntities();
    void FindVisibleMeshEntities(const Math::Frustum & frustum, std::vector<MeshComponent*>* pList);
};

} // namespace Scene
} // namespace NFE
