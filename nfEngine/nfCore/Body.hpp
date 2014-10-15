/**
    NFEngine project

    \file   Body.hpp
    \brief  Body component declaration.
*/

#pragma once

#include "Core.hpp"
#include "Component.hpp"
#include "CollisionShape.hpp"

class  btDefaultMotionState;
class  btRigidBody;

namespace NFE {
namespace Scene {

/**
 * Body entity descriptor used for serialization.
 */
#pragma pack(push, 1)
struct BodyComponentDesc
{
    float mass;
    char collisionShapeName[RES_NAME_MAX_LENGTH];
};
#pragma pack(pop)

/**
 * Component representing physical body.
 */
class CORE_API BodyComponent : public Component
{
    friend class SceneManager;
    friend class Entity;

    Resource::CollisionShape* mCollisionShape;

    // TODO: these shouldn't be allocated dynamically
    btDefaultMotionState* mMotionState;
    btRigidBody* mBody;

    // physical properties
    float mMass;

    // disable unwanted methods
    BodyComponent(const BodyComponent&);
    BodyComponent& operator=(const BodyComponent&);

    void OnUpdate(float dt);
    void OnMove();

public:
    BodyComponent(Entity* pParent);
    ~BodyComponent();

    /**
     * Enable physics interaction via assigning a collision shape
     */
    void EnablePhysics(Resource::CollisionShape* pShape);

    /**
     * Disable physics interaction
     */
    void DisablePhysics();

    /**
     * Set body mass
     * @param mass New mass value. When less or equal zero, the body is static.
     */
    void SetMass(float mass);

    float GetMass();

    Result Deserialize(Common::InputStream* pStream);
    Result Serialize(Common::OutputStream* pStream) const;
    void ReceiveMessage(ComponentMsg type, void* pData);
};

} // namespace Scene
} // namespace NFE
