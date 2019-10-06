/**
 * @file
 * @brief  Declaration of reflection system's base object class.
 */

#pragma once

#include "../nfCommon.hpp"


namespace NFE {

/**
 * Base object class for all polymorphic types registered in RTTI.
 */
class NFCOMMON_API IObject
{
public:
    virtual ~IObject() { }

    virtual const RTTI::Type* GetDynamicType() const = 0;
};

static_assert(sizeof(IObject) == sizeof(size_t), "IObject should not have any members");

} // namespace NFE
