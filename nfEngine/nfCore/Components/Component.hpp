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
    static std::vector<ComponentInfo> gComponents;

protected:
    static int gComponentIdCounter;

public:
    virtual ~Component() {}

    virtual void Invalidate();

    /**
     * Register a component type. Should be called via @p NFE_REGISTER_COMPONENT macro.
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
public:
    virtual ~ComponentBase() {}

    /**
     * Get component's class unique ID.
     */
    static int GetID()
    {
        static int id = gComponentIdCounter++;
        return id;
    }
};

#define NFE_REGISTER_COMPONENT(c) \
    static bool gRegisterComponent_##c##_Var = Component::Register(c::GetID(), #c, sizeof(c));

} // namespace Scene
} // namespace NFE
