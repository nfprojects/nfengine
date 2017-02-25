/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of reflection system's Type class.
 */

#include "PCH.hpp"
#include "ReflectionType.hpp"
#include "../../System/Assertion.hpp"
#include "../../Memory/DefaultAllocator.hpp"
#include "../../Logger/Logger.hpp"
#include "../../Config/Config.hpp"
#include "../../Config/ConfigValue.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;


Type::Type(const TypeInfo& info)
    : mInitialized(false)
{
    if (info.name)
    {
        mName = info.name;
    }

    NFE_ASSERT(info.kind != TypeKind::Undefined, "Type kind cannot be undefined");
    mKind = info.kind;

    // downcast to 4 bytes to save space
    NFE_ASSERT(info.size < UINT32_MAX, "Type size is too big");
    NFE_ASSERT(info.alignment < UINT32_MAX, "Type size is too big");

    mSize = static_cast<uint32>(info.size);
    mAlignment = static_cast<uint32>(info.alignment);
}


void* Type::CreateObject() const
{
    void* object = NFE_MALLOC(mSize, mAlignment);
    if (!object)
    {
        return nullptr;
    }

    // TODO call the constructor

    return object;
}

} // namespace RTTI
} // namespace NFE
