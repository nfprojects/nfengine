/**
    NFEngine project

    \file   Component.h
    \brief  Component's base class declarations.
*/

#pragma once
#include "Core.h"
#include "Aligned.h"

namespace NFE {
namespace Scene {

/**
 * Built-in component types.
 */
enum class ComponentType
{
    Unknown = 0,
    Mesh,       // render mesh
    Light,      // emit light
    Physics,    // enable physics simulation and collision detection. Max one per entity!
    Camera,     // can be used as view during scene rendering

    // FUTURE:
    Sound,      // sound source
    Emitter,    // particles emitter
    Trigger,    // collision trigger
    Script,
};

/**
 * Predefined component message types.
 */
enum class ComponentMsg
{
    OnMove = 0,
    OnSetVelocity,
    OnSetAngularVelocity
};


/**
 * Base abstract class for entity component.
 */
NFE_ALIGN(16)
class CORE_API Component : public Util::Aligned
{
    friend class SceneManager;
    friend class Entity;

    // disable some methods
    Component& operator=(const Component&);
    Component(const Component&);

protected:
    ComponentType mType;
    Entity* mOwner;

    /**
     * Virtual component is a component created and used by another component.
     * For example: light component creates camera components used to shadows rendering.
     * Serialization mechanism takes into account only non-virtual components.
     */
    bool mVirtual;

public:
    Component(Entity* pParent);
    virtual ~Component();

    /**
     * Component serialization routine. Must be implemented by every component.
     * @return Result::OK on success.
     */
    virtual Result Serialize(Common::OutputStream* pStream) const = 0;

    /**
     * Component deserialization routine. Must be implemented by every component.
     * @return Result::OK on success.
     */
    virtual Result Deserialize(Common::InputStream* pStream) = 0;

    /**
     * Method called on message arrival.
     */
    virtual void ReceiveMessage(ComponentMsg type, void* pData);

    __forceinline ComponentType GetType() const
    {
        return mType;
    }

    /**
     * Get owner entity.
     * @return NULL if does not belong to any entity.
     */
    __forceinline Entity* GetOwner() const
    {
        return mOwner;
    }
};

} // namespace Scene
} // namespace NFE
