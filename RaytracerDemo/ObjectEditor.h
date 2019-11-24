#pragma once

#include "../nfEngine/nfCommon/Reflection/ReflectionTypeRegistry.hpp"
#include "../nfEngine/nfCommon/Reflection/Types/ReflectionFundamentalType.hpp"
#include "../nfEngine/nfCommon/Reflection/Object.hpp"

namespace NFE {

bool EditObject_Root_Internal(const char* rootName, const RTTI::Type* type, void* data);

template <typename T>
typename std::enable_if<std::is_base_of_v<IObject, T>, const RTTI::Type*>::type GetEditedObjectType(const T& object)
{
    return object.GetDynamicType();
}

template <typename T>
typename std::enable_if<!std::is_base_of_v<IObject, T>, const RTTI::Type*>::type GetEditedObjectType(const T& object)
{
    return RTTI::GetType<T>();
}

template<typename T>
bool EditObject(const char* rootName, T& object)
{
    const RTTI::Type* type = GetEditedObjectType<T>(object);
    return EditObject_Root_Internal(rootName, type, &object);
}

} // namesapce NFE