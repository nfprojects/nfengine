/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"

#include "../../../Common/Reflection/ReflectionClassDeclare.hpp"
#include "../../../Common/Reflection/Object.hpp"
#include "../../../Common/Containers/SharedPtr.hpp"

namespace NFE {
namespace Scene {

/**
 * Base class for all event types.
 */
class CORE_API Event : public IObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(Event)

public:
    virtual ~Event() { }
};

using EventPtr = Common::SharedPtr<Event>;


} // namespace Scene
} // namespace NFE
