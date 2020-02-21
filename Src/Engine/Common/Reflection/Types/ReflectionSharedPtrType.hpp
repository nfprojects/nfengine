/**
 * @file
 * @brief  Definition of SharedPtrType.
 */

#pragma once

#include "ReflectionPointerType.hpp"
#include "../../Config/Config.hpp"
#include "../../Config/ConfigValue.hpp"
#include "../../Containers/SharedPtr.hpp"

namespace NFE {
namespace RTTI {

template<typename T>
class SharedPtrTypeImpl final : public PointerType
{
public:
    using ObjectType = Common::SharedPtr<T>;

    SharedPtrTypeImpl(const TypeInfo& info)
        : PointerType(info)
    {
        mPointedType = GetType<T>();
        NFE_ASSERT(mPointedType, "Invalid pointed type");
    }

    virtual void* GetPointedData(const void* sharedPtrObject) const override
    {
        NFE_ASSERT(sharedPtrObject, "Trying to access nullptr");
        const ObjectType& typedObject = *static_cast<const ObjectType*>(sharedPtrObject);
        return typedObject.Get();
    }

    virtual const Type* GetPointedType(const void* sharedPtrObject) const override
    {
        NFE_ASSERT(sharedPtrObject, "Trying to access nullptr");
        const ObjectType& typedObject = *static_cast<const ObjectType*>(sharedPtrObject);

        if (typedObject)
        {
            if (mPointedType->GetKind() == TypeKind::AbstractClass || mPointedType->GetKind() == TypeKind::PolymorphicClass)
            {
                return reinterpret_cast<const IObject*>(typedObject.Get())->GetDynamicType();
            }
            else
            {
                return mPointedType;
            }
        }

        return nullptr;
    }

    virtual void Reset(void* sharedPtrObject, const Type* newDataType) const override
    {
        NFE_ASSERT(sharedPtrObject, "Trying to access nullptr");
        ObjectType& typedObject = *static_cast<ObjectType*>(sharedPtrObject);
        if (newDataType)
        {
            typedObject.Reset(newDataType->CreateObject<T>());
        }
        else
        {
            typedObject.Reset();
        }
    }

    bool Serialize(const void* object, Common::Config& config, Common::ConfigValue& outValue) const override
    {
        NFE_ASSERT(object, "Trying to serialize nullptr");

        NFE_UNUSED(object);
        NFE_UNUSED(config);
        NFE_UNUSED(outValue);

        NFE_ASSERT(false, "Not implemented!");

        return false;
    }

    bool Deserialize(void* outObject, const Common::Config& config, const Common::ConfigValue& value) const override
    {
        NFE_ASSERT(outObject, "Trying to deserialize to nullptr");

        NFE_UNUSED(outObject);
        NFE_UNUSED(config);
        NFE_UNUSED(value);

        NFE_ASSERT(false, "Not implemented!");

        return false;
    }
};


/**
 * Generic type creator for SharedPtr<T> types.
 */
template<typename T>
class TypeCreator<Common::SharedPtr<T>>
{
public:
    using TypeClass = SharedPtrTypeImpl<T>;
    using ObjectType = Common::SharedPtr<T>;

    static Type* CreateType()
    {
        const Type* templateArgumentType = GetType<T>();

        const Common::String typeName = Common::String("NFE::Common::SharedPtr<") + templateArgumentType->GetName() + '>';

        TypeInfo typeInfo;
        typeInfo.kind = TypeKind::SharedPtr;
        typeInfo.size = sizeof(ObjectType);
        typeInfo.alignment = alignof(ObjectType);
        typeInfo.name = typeName.Str();
        typeInfo.constructor = []() { return new ObjectType; };
        typeInfo.arrayConstructor = [](uint32 num) { return new ObjectType[num]; };

        return new SharedPtrTypeImpl<T>(typeInfo);
    }
};


} // namespace RTTI
} // namespace NFE
