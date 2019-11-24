/**
 * @file
 * @brief   Definition of reflection system's base object class.
 */

#include "PCH.hpp"
#include "Object.hpp"

namespace NFE {

IObject::~IObject() = default;

bool IObject::OnPropertyChanged(const Common::StringView propertyName)
{
    NFE_UNUSED(propertyName);
    return true;
}

} // namespace NFE
