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


class CORE_API Component
{
protected:
    static int gComponentIdCounter;

public:
    virtual ~Component() {}

    virtual void Invalidate();
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


} // namespace Scene
} // namespace NFE
