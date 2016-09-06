/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Component's base class declarations.
 */

#pragma once

#include "../Core.hpp"
#include "../Scene/Entity.hpp"
#include "nfCommon/nfCommon.hpp"

#include <vector>


namespace NFE {
namespace Scene {

#define NFE_MAX_COMPONENT_TYPES     32
#define NFE_INVALID_COMPONENT_TYPE  0xFFFFFFFF

/**
 * Structure storing component type information.
 */
struct ComponentInfo
{
    int id;
    size_t size;
    const char* name;
};

class CORE_API Component
{
private:
    // global list of component types
    static std::vector<ComponentInfo> mComponentTypes;

    Entity*         mEntity;            // parent entity
    int             mComponentType;     // component type

    // called by Entity when the component is attached
    void OnAttachToEntity(Entity* entity);

protected:
    // global component type ID counter
    static int mComponentIdCounter;

    /**
     * Should be called by the component implementation when a property has changed,
     * so the systems are notified.
     */
    void OnChanged();

public:
    Component(int id);
    virtual ~Component();

    /**
     * Get parent Entity pointer.
     */
    NFE_INLINE Entity* GetEntity() const { return mEntity; }

    /**
     * Register a component type. Should be called via @p NFE_REGISTER_COMPONENT macro.
     * @param id   Component's unique ID.
     * @param name Component name.
     * @param size Component class size in bytes.
     * @return True on success.
     */
    static bool RegisterType(int id, const char* name, size_t size);
};

/**
 * Base class for entity component.
 *
 * All components must derive from this class in order to properly generate component IDs
 * thanks to "curiously recurring template pattern".
 */
template<typename T>
class ComponentBase : public Component
{
    NFE_MAKE_NONCOPYABLE(ComponentBase);

public:
    NFE_INLINE ComponentBase()
        : Component(GetID())
    { }

    virtual ~ComponentBase() {}

    /**
     * Get component's class unique ID.
     */
    static int GetID()
    {
        static int id = mComponentIdCounter++;
        return id;
    }
};

/**
 * This macro should be used to register an existence of a component type (ususally in *.cpp file).
 * Component registering is required by EntityManager for memory allocations.
 */
#define NFE_REGISTER_COMPONENT(c) \
    static bool gRegisterComponent_##c##_Var = Component::RegisterType(c::GetID(), #c, sizeof(c));

} // namespace Scene
} // namespace NFE
