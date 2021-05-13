#pragma once

#include "Engine/Common/Reflection/ReflectionTypeRegistry.hpp"
#include "Engine/Common/Reflection/Types/ReflectionFundamentalType.hpp"
#include "Engine/Common/Reflection/Object.hpp"
#include "Engine/Common/Reflection/ReflectionVariant.hpp"
#include "Engine/Common/Reflection/ReflectionMemberPath.hpp"

namespace NFE {

struct EditObjectContext
{
    bool readOnly = false;

    RTTI::MemberPath modifiedPath;
    RTTI::Variant objectAfter;
};

bool EditObject_Root_Internal(const char* rootName, const RTTI::Type* type, const void* data, EditObjectContext& context);
bool ApplyObjectChanges_Internal(const RTTI::Type* type, void* data, const EditObjectContext& context);

template <typename T>
typename std::enable_if<std::is_base_of_v<IObject, T>, const RTTI::Type*>::type GetEditedObjectType(const T& object)
{
    return object.GetDynamicType();
}

template <typename T>
typename std::enable_if<!std::is_base_of_v<IObject, T>, const RTTI::Type*>::type GetEditedObjectType(const T& object)
{
    NFE_UNUSED(object);
    return RTTI::GetType<T>();
}



template<typename T>
bool EditObject(const char* rootName, const T& object, EditObjectContext& context)
{
    const RTTI::Type* type = GetEditedObjectType<T>(object);
    return EditObject_Root_Internal(rootName, type, &object, context);
}

template<typename T>
bool ApplyObjectChanges(T& object, const EditObjectContext& context)
{
    const RTTI::Type* type = GetEditedObjectType<T>(object);
    return ApplyObjectChanges_Internal(type, &object, context);
}

} // namesapce NFE