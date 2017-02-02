/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"

#include "nfCommon/Reflection/ReflectionMacros.hpp"


namespace NFE {
namespace Scene {

/**
 * Base class for all event types.
 */
class CORE_API Event
{
    NFE_DECLARE_POLYMORPHIC_CLASS(Event);

public:
    virtual ~Event() { }
};

} // namespace Scene
} // namespace NFE

NFE_DECLARE_TYPE(NFE::Scene::Event);
