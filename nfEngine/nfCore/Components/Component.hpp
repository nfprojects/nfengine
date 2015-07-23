/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Component's base class declarations.
 */

#pragma once

#include "../Core.hpp"
#include "../Aligned.hpp"

namespace NFE {
namespace Scene {

/**
 * Base abstract class for entity component.
 * All components must derive from this class.
 */
NFE_ALIGN16
class CORE_API Component
{
    static int gComponentIdCounter;

public:
    virtual ~Component() = 0;

    /**
     * Get component's class unique ID.
     */
    template <typename Type>
    static int GetID()
    {
        static int id = gComponentIdCounter++;
        return id;
    }
};

} // namespace Scene
} // namespace NFE
