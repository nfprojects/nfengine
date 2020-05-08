/**
 * @file
 * @brief  Declaration of reflection system's base object class.
 */

#pragma once

#include "ReflectionClassDeclare.hpp"


namespace NFE {

/**
 * Base object class for all polymorphic types registered in RTTI.
 */
class NFCOMMON_API IObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(IObject)

public:
    virtual ~IObject();

    // Called when a property gets changed (e.g. by the editor)
    virtual bool OnPropertyChanged(const Common::StringView propertyName);
};

using ObjectPtr = Common::SharedPtr<IObject>;

static_assert(sizeof(IObject) == sizeof(size_t), "IObject should not have any members");

} // namespace NFE
