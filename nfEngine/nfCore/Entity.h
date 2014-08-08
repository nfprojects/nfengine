/**
    NFEngine project

    \file   Entity.h
    \brief  Scene entity declarations.
*/

#pragma once

#include "Core.h"
#include "Aligned.h"

namespace NFE {
namespace Scene {

#define ENTITY_FLAG_IS_ROOT (1 << 0)
#define ENTITY_FLAG_MOVED (1 << 1)

// TODO: move to nfCommon, rename to Matrix3x3
NFE_ALIGNED_CLASS(XOrientation)
{
public:
    Math::Vector x, y, z;

    XOrientation()
    {
        x = Math::Vector(1.0f, 0.0f, 0.0f, 0.0f);
        y = Math::Vector(0.0f, 1.0f, 0.0f, 0.0f);
        z = Math::Vector(0.0f, 0.0f, 1.0f, 0.0f);
    }

    XOrientation(const Math::Matrix & mat)
    {
        x = mat.r[0];
        y = mat.r[1];
        z = mat.r[2];
    }
};


// structure used for serialization
#pragma pack(push, 1)
struct EntityDesc
{
    Math::Float3 pos;     // local position
    Math::Float4 quat;    // local orientation
    uint16 compNum;       // number of components
    uint16 childrenNum;   // number of children entities
};
#pragma pack(pop)


NFE_ALIGN(16)
class CORE_API Entity : public Util::Aligned
{
    friend class SceneManager;
    friend class Segment;
    friend class Component;
    friend class MeshComponent;
    friend class LightComponent;
    friend class BodyComponent;
    friend class Camera;

    //disable unwanted methods
    Entity(const Entity&);
    Entity& operator=(const Entity&);

    /*
        first 3 rows - orientation
        last row - translation

        GlobalMatrix = LocalMatrix * pParent->GlobalMatrix
    */
    Math::Matrix mMatrix, mLocalMatrix;

    // velocities
    Math::Vector mVelocity, mAngularVelocity;


    /// TODO: These 3 pointer can be reducted to one:
    SceneManager* mScene;         // parent scene
    Segment* mSegment;    // parent scene segment
    Entity* mParent;          // parent entity

    // list of enabled components
    std::set<Component*> mComponents;

    // list of child entities
    std::set<Entity*> mChildren;


    // Entity unique name.
    char* mName;

    uint32 mFlags;
    void* mUserPointer;

    // called when global position has changed
    void UpdateLocalMatrix();

    // called when local position has changed
    void UpdateGlobalMatrix();

public:
    Entity();
    virtual ~Entity();

    const char* GetName() const;
    Result SetName(const char* pNewName);

    void SetUserPointer(void* ptr);
    void* GetUserPointer() const;

    SceneManager* GetScene() const;
    Segment* GetSceneSegment() const;
    Entity* GetParent() const;

    /**
     * Attach an entity as a child.
     * @return Result::OK on success, see logs on failure.
     */
    Result Attach(Entity* pChild);

    /**
     * Detach a child entity.
     * @return Result::OK on success, see logs on failure.
     */
    Result Detach(Entity* pChild);


    /// Global transformation

    /**
     * Set global position and orientation as 4x4 matrix.
     */
    void SetMatrix(const Math::Matrix& matrix);

    /**
     * Get global position and orientation as 4x4 matrix.
     */  
    Math::Matrix GetMatrix() const;

    /**
     * Set global position.
     */
    void SetPosition(const Math::Vector& pos);

    /**
     * Get global position in the scene.
     */
    Math::Vector GetPosition() const;

    /**
     * Set global orientation using quaternion.
     */
    void SetOrientation(const Math::Quaternion& quat);

    /**
     * Set global orientation using rotation matrix.
     */
    void SetOrientation(const XOrientation* pOrientation);

    /**
     * Get global orientation.
     */
    void GetOrientation(XOrientation* pOrientation) const;



    /// Local transformation

    /**
     * Set position relative to parent entity.
     */
    void SetLocalPosition(const Math::Vector& pos);

    /**
     * Get position relative to parent entity.
     */
    Math::Vector GetLocalPosition() const;

    /**
     * Set orientation relative to parent entity.
     */
    void SetLocalOrientation(const XOrientation* pOrientation);

    /**
     * Get current orientation relative to parent entity.
     * @param[out] pOrientation Pointer to the result
     */
    void GetLocalOrientation(XOrientation* pOrientation) const;



    /**
     * Get current linear velocity.
     */
    Math::Vector GetVelocity() const;

    /**
     * Get current angular velocity (in radians per second).
     */
    Math::Vector GetAngularVelocity() const;

    /**
     * Change linear velocity.
     */
    void SetVelocity(const Math::Vector& newVelocity);

    /**
     * Change angular velocity (in radians per second). 
     */
    void SetAngularVelocity(const Math::Vector& newAngularVelocity);

    /**
     * Called when the entity is moved by user or physics engine
     */
    void OnMove();


    // Components management

    /**
     * Get a component by type.
     */
    Component* GetComponent(ComponentType type) const;
    Result AddComponent(Component* pComponent);
    Result RemoveComponent(Component* pComponent);

    /**
     * Remove and destroy all the entity components.
     */
    void RemoveAllComponents();

    /**
     * Send a message to all the components
     * @param type Message type
     * @param pData Message data
     */
    void BroadcastMessage(ComponentMsg type, void* pData);

    // Called by the engine when the entity is spawned due to scene segment creation.
    Result Deserialize(Common::InputStream* pStream, const Math::Vector& offset);

    // Called by the engine when the entity is removed due to scene segment deletion.
    Result Serialize(Common::OutputStream* pStream, const Math::Vector& offset) const;
};

} // namespace Scene
} // namespace NFE
