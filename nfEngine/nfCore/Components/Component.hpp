/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Component's base class declarations.
 */

#pragma once

#include "../Core.hpp"

namespace NFE {
namespace Scene {

#define NFE_MAX_COMPONENT_TYPES 32

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
    static std::vector<ComponentInfo> mComponents;

protected:
    static int mComponentIdCounter;

public:
    virtual ~Component() {}

    /**
     * Invalidate a component. This should be called when entity is moved to removed components
     * list.
     */
    virtual void Invalidate();

    /**
     * Register a component type. Should be called via @p NFE_REGISTER_COMPONENT macro.
     * @param id   Component's unique ID.
     * @param name Component name.
     * @param size Component class size in bytes.
     * @return True on success.
     */
    static bool Register(int id, const char* name, size_t size);
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
    ComponentBase(const ComponentBase&) = delete;
    ComponentBase& operator=(const ComponentBase&) = delete;

public:
    ComponentBase() { }

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
    static bool gRegisterComponent_##c##_Var = Component::Register(c::GetID(), #c, sizeof(c));

} // namespace Scene
} // namespace NFE
