/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of reflection system's EnumType class.
 */

#include "PCH.hpp"
#include "ReflectionEnumType.hpp"

#include "../../System/Assertion.hpp"
#include "../../Memory/DefaultAllocator.hpp"
#include "../../Logger/Logger.hpp"
#include "../../Config/Config.hpp"
#include "../../Config/ConfigValue.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;


EnumType::EnumType(const EnumTypeInfo& info)
    : Type(info)
    , mOptions(info.options)
{
    NFE_ASSERT(mOptions.Size() > 0, "Enum type must have at least one option");
}

/*
void EnumType::PrintInfo() const
{
    Type::PrintInfo();

#ifdef _DEBUG
    {
        if (mParent)
        {
            LOG_DEBUG("    - Parent type: %s", mParent->GetName());
        }

        for (const Member& member : mMembers)
        {
            LOG_DEBUG("    - Member '%s': type=%s, offset=%u", member.GetName(), member.GetType()->GetName(), member.GetOffset());
        }
    }
#endif // _DEBUG
}
*/

bool EnumType::Serialize(const void* object, Config& config, ConfigValue& outValue) const
{

}

bool EnumType::Deserialize(void* outObject, const Config& config, const ConfigValue& value) const
{

}

} // namespace RTTI
} // namespace NFE
