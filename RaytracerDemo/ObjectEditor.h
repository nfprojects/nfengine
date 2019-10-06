#pragma once

#include "../nfEngine/nfCommon/Reflection/ReflectionTypeRegistry.hpp"
#include "../nfEngine/nfCommon/Reflection/Types/ReflectionFundamentalType.hpp"

namespace NFE {

bool EditObject_Root_Internal(const char* rootName, const RTTI::Type* type, void* data);

template<typename T>
bool EditObject(const char* rootName, T& object)
{
    const RTTI::Type* type = RTTI::GetType<T>();
    return EditObject_Root_Internal(rootName, type, &object);
}

} // namesapce NFE