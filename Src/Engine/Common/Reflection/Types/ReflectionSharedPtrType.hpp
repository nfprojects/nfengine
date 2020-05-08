/**
 * @file
 * @brief  Definition of SharedPtrType.
 */

#pragma once

#include "ReflectionPointerType.hpp"
#include "../ReflectionTypeResolver.hpp"
#include "../../Containers/SharedPtr.hpp"

namespace NFE {
namespace RTTI {


// base class for all shared pointer types
class NFCOMMON_API SharedPtrType : public PointerType
{
public:
    SharedPtrType(const TypeInfo& info, const Type* underlyingType);

    // reset with an existing IObject
    virtual void Assign(void* sharedPtrObject, const Common::SharedPtr<IObject>& newPtr) const = 0;

    virtual bool Serialize(const void* object, Common::IConfig& config, Common::ConfigValue& outValue, SerializationContext& context) const override final;
    virtual bool Deserialize(void* outObject, const Common::IConfig& config, const Common::ConfigValue& value, const SerializationContext& context) const override final;
    virtual bool SerializeBinary(const void* object, Common::OutputStream* stream, SerializationContext& context) const override final;
    virtual bool DeserializeBinary(void* outObject, Common::InputStream& stream, const SerializationContext& context) const override final;
};


template<typename T>
class SharedPtrTypeImpl final : public SharedPtrType
{
public:
    using ObjectType = Common::SharedPtr<T>;

    static_assert(std::is_base_of_v<IObject, T>, "Reflection: SharedPtr must point to IObject-based class");

    SharedPtrTypeImpl(const TypeInfo& info)
        : SharedPtrType(info, GetType<T>())
    { }

    virtual void* GetPointedData(const void* ptrObject) const override
    {
        NFE_ASSERT(ptrObject, "Trying to access nullptr");
        const ObjectType& typedObject = *static_cast<const ObjectType*>(ptrObject);
        return typedObject.Get();
    }

    virtual const Type* GetPointedDataType(const void* ptrObject) const override
    {
        NFE_ASSERT(ptrObject, "Trying to access nullptr");
        const ObjectType& typedObject = *static_cast<const ObjectType*>(ptrObject);

        if (typedObject)
        {
            if (mUnderlyingType->GetKind() == TypeKind::AbstractClass || mUnderlyingType->GetKind() == TypeKind::PolymorphicClass)
            {
                return typedObject.Get()->GetDynamicType();
            }
            else
            {
                return mUnderlyingType;
            }
        }

        return nullptr;
    }

    virtual void Assign(void* sharedPtrObject, const Common::SharedPtr<IObject>& newPtr) const
    {
        NFE_ASSERT(sharedPtrObject, "Trying to access nullptr");

        ObjectType& typedObject = *static_cast<ObjectType*>(sharedPtrObject);
        typedObject = newPtr;
    }

    virtual void Reset(void* ptrObject, const Type* newDataType) const override
    {
        NFE_ASSERT(ptrObject, "Trying to access nullptr");
        ObjectType& typedObject = *static_cast<ObjectType*>(ptrObject);
        if (newDataType)
        {
            typedObject.Reset(newDataType->CreateObject<T>());
        }
        else
        {
            typedObject.Reset();
        }
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
