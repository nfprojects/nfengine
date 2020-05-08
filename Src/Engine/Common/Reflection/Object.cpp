/**
 * @file
 * @brief   Definition of reflection system's base object class.
 */

#include "PCH.hpp"
#include "Object.hpp"
#include "ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::IObject)
NFE_END_DEFINE_CLASS()


namespace NFE {

IObject::~IObject() = default;

bool IObject::OnPropertyChanged(const Common::StringView propertyName)
{
    NFE_UNUSED(propertyName);
    return true;
}

} // namespace NFE
