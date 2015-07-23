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
 */
NFE_ALIGN16
class CORE_API Component : public Util::Aligned
{
    static int gComponentIdCounter;

public:
    template <typename Type>
    static int GetID()
    {
        static id = gComponentIdCounter++;
        return id;
    }
};

} // namespace Scene
} // namespace NFE
