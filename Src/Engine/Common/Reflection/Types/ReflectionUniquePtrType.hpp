/**
 * @file
 * @author Witek902
 * @brief  Definition of UniquePtrType.
 */

#pragma once

#include "ReflectionPointerType.hpp"
#include "ReflectionClassType.hpp" // TODO remove
#include "../ReflectionTypeResolver.hpp"
#include "../../Config/ConfigInterface.hpp"
#include "../../Config/ConfigValue.hpp"
#include "../../Containers/UniquePtr.hpp"

// TODO remove
#include "../../Logger/Logger.hpp"

namespace NFE {
namespace RTTI {


// base class for all unique pointer types
class NFCOMMON_API UniquePtrType : public PointerType
{
public:
    NFE_FORCE_INLINE UniquePtrType(const TypeInfo& info) : PointerType(info) { }

    virtual bool SerializeBinary(const void* object, Common::OutputStream* stream, SerializationContext& context) const override final;
    virtual bool DeserializeBinary(void* outObject, Common::InputStream& stream, const SerializationContext& context) const override final;
};


template<typename T>
class UniquePtrTypeImpl final : public UniquePtrType
{
public:
    using ObjectType = Common::UniquePtr<T>;

    UniquePtrTypeImpl(const TypeInfo& info)
        : UniquePtrType(info)
    {
        mUnderlyingType = GetType<T>();
        NFE_ASSERT(mUnderlyingType, "Invalid pointed type");
    }

    virtual void* GetPointedData(const void* uniquePtrObject) const override
    {
        NFE_ASSERT(uniquePtrObject, "Trying to access nullptr");
        const ObjectType& typedObject = *static_cast<const ObjectType*>(uniquePtrObject);
        return typedObject.Get();
    }

    virtual const Type* GetPointedDataType(const void* uniquePtrObject) const override
    {
        NFE_ASSERT(uniquePtrObject, "Trying to access nullptr");
        const ObjectType& typedObject = *static_cast<const ObjectType*>(uniquePtrObject);

        if (typedObject)
        {
            if (mUnderlyingType->GetKind() == TypeKind::AbstractClass || mUnderlyingType->GetKind() == TypeKind::PolymorphicClass)
            {
                return BitCast<const IObject*>(typedObject.Get())->GetDynamicType();
            }
            else
            {
                return mUnderlyingType;
            }
        }

        return nullptr;
    }

    virtual void Reset(void* uniquePtrObject, const Type* newDataType) const override
    {
        NFE_ASSERT(uniquePtrObject, "Trying to access nullptr");
        ObjectType& typedObject = *static_cast<ObjectType*>(uniquePtrObject);
        if (newDataType)
        {
            typedObject.Reset(newDataType->CreateObject<T>());
        }
        else
        {
            typedObject.Reset();
        }
    }

    bool Serialize(const void* object, Common::IConfig& config, Common::ConfigValue& outValue, SerializationContext& context) const override
    {
        NFE_ASSERT(object, "Trying to serialize nullptr");
        const ObjectType& typedObject = *static_cast<const ObjectType*>(object);

        if (typedObject)
        {
            mUnderlyingType->Serialize(typedObject.Get(), config, outValue, context);
        }
        else // null pointer
        {
            outValue = Common::ConfigValue(0);
        }

        return true;
    }

    bool Deserialize(void* outObject, const Common::IConfig& config, const Common::ConfigValue& value, const SerializationContext& context) const override
    {
        NFE_ASSERT(outObject, "Trying to deserialize to nullptr");
        ObjectType& typedObject = *static_cast<ObjectType*>(outObject);

        if (value.Is<int32>()) // nullptr
        {
            if (value.Get<int32>() != 0)
            {
                NFE_LOG_WARNING("Expected zero");
            }

            typedObject.Reset();
            return true;
        }
        else if (value.IsObject()) // valid object
        {
            const char* typeName = nullptr;
            const auto configIteratorCallback = [&typeName](Common::StringView key, const Common::ConfigValue& value)
            {
                // marker found
                if (key == ClassType::TYPE_MARKER)
                {
                    if (!value.IsString())
                    {
                        NFE_LOG_ERROR("Marker type found - string expected");
                        return false;
                    }

                    typeName = value.Get<const char*>();
                    return false;
                }

                return true;
            };

            // extract target object type from marker
            config.Iterate(configIteratorCallback, value.GetObj());

            const Type* targetType = nullptr;
            if (typeName)
            {
                // get type from name
                targetType = ITypeRegistry::GetInstance().GetExistingType(typeName);
                if (!targetType)
                {
                    NFE_LOG_ERROR("Type not found: '%s'", typeName);
                    return false;
                }

                if (!targetType->IsA(mUnderlyingType))
                {
                    const Common::StringView name = mUnderlyingType->GetName();
                    NFE_LOG_ERROR("Target type '%s' is not related with pointed type '%.*s'", typeName, name.Length(), name.Data());
                    return false;
                }
            }
            else
            {
                if (mUnderlyingType->GetKind() != TypeKind::AbstractClass)
                {
                    NFE_LOG_WARNING("Type marker not found - using pointed type as a reference");
                    targetType = mUnderlyingType;
                }
                else // pointed type is abstract class
                {
                    NFE_LOG_ERROR("Type marker not found - cannot resolve target type");
                    return false;
                }
            }

            if (!targetType->IsConstructible())
            {
                NFE_LOG_ERROR("Target type '%s' is not constructible", typeName);
                return false;
            }

            // construct the object & assign to the smart pointer
            typedObject.Reset(targetType->CreateObject<T>());
            if (!typedObject)
            {
                NFE_LOG_ERROR("Failed to create object");
                return false;
            }

            // deserialize the object
            return targetType->Deserialize(typedObject.Get(), config, value, context);
        }

        NFE_LOG_ERROR("Expected zero (nullptr) or an object");
        return false;
    }
};


/**
 * Generic type creator for UniquePtr<T> types.
 */
template<typename T>
class TypeCreator<Common::UniquePtr<T>>
{
public:
    using TypeClass = UniquePtrTypeImpl<T>;
    using ObjectType = Common::UniquePtr<T>;

    static Type* CreateType()
    {
        const Type* templateArgumentType = GetType<T>();

        const Common::String typeName = Common::String("NFE::Common::UniquePtr<") + templateArgumentType->GetName() + '>';

        TypeInfo typeInfo;
        typeInfo.kind = TypeKind::UniquePtr;
        typeInfo.size = sizeof(ObjectType);
        typeInfo.alignment = alignof(ObjectType);
        typeInfo.name = typeName.Str();
        typeInfo.constructor = []() { return new ObjectType; };
        typeInfo.arrayConstructor = [](uint32 num) { return new ObjectType[num]; };

        return new UniquePtrTypeImpl<T>(typeInfo);
    }
};


} // namespace RTTI
} // namespace NFE
