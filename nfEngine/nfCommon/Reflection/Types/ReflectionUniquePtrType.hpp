/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of UniquePtrType.
 */

#pragma once

#include "ReflectionType.hpp"
#include "../../Config/Config.hpp"
#include "../../Config/ConfigValue.hpp"
#include "../../Containers/UniquePtr.hpp"

namespace NFE {
namespace RTTI {

/**
 * Type information for UniquePtr<T> types.
 */
class NFCOMMON_API UniquePtrType : public Type
{
    NFE_MAKE_NONCOPYABLE(UniquePtrType)

public:
    NFE_FORCE_INLINE const Type* GetPointedType() const { return mPointedType; }

    // get data pointed by the unique pointer
    virtual void* GetPointedData(const void* uniquePtrObject) const = 0;

    // get object type under the pointer
    virtual const Type* GetPointedType(const void* uniquePtrObject) const = 0;

    // set object under the pointer
    virtual void Reset(void* uniquePtrObject, const Type* newDataType = nullptr) const = 0;

    virtual bool Compare(const void* objectA, const void* objectB) const override;

protected:
    NFE_FORCE_INLINE UniquePtrType(const TypeInfo& info) : Type(info) { }

    const Type* mPointedType;
};

template<typename T>
class UniquePtrTypeImpl final : public UniquePtrType
{
public:
    using ObjectType = Common::UniquePtr<T>;

    NFE_FORCE_INLINE UniquePtrTypeImpl(const TypeInfo& info)
        : UniquePtrType(info)
    {
        mPointedType = GetType<T>();
        NFE_ASSERT(mPointedType, "Invalid pointed type");
    }

    virtual void* GetPointedData(const void* uniquePtrObject) const override
    {
        NFE_ASSERT(uniquePtrObject, "Trying to access nullptr");
        const ObjectType& typedObject = *static_cast<const ObjectType*>(uniquePtrObject);
        return typedObject.Get();
    }

    virtual const Type* GetPointedType(const void* uniquePtrObject) const override
    {
        NFE_ASSERT(uniquePtrObject, "Trying to access nullptr");
        const ObjectType& typedObject = *static_cast<const ObjectType*>(uniquePtrObject);

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

    bool Serialize(const void* object, Common::Config& config, Common::ConfigValue& outValue) const override
    {
        NFE_ASSERT(object, "Trying to serialize nullptr");
        const ObjectType& typedObject = *static_cast<const ObjectType*>(object);

        if (typedObject)
        {
            mPointedType->Serialize(typedObject.Get(), config, outValue);
        }
        else // null pointer
        {
            outValue = Common::ConfigValue(0);
        }

        return true;
    }

    bool Deserialize(void* outObject, const Common::Config& config, const Common::ConfigValue& value) const override
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
                targetType = TypeRegistry::GetInstance().GetExistingType(Common::StringView(typeName));
                if (!targetType)
                {
                    NFE_LOG_ERROR("Type not found: '%s'", typeName);
                    return false;
                }

                if (!targetType->IsA(mPointedType))
                {
                    NFE_LOG_ERROR("Target type '%s' is not related with pointed type '%s'", typeName, mPointedType->GetName().Str());
                    return false;
                }
            }
            else
            {
                if (mPointedType->GetKind() != TypeKind::AbstractClass)
                {
                    NFE_LOG_WARNING("Type marker not found - using pointed type as a reference");
                    targetType = mPointedType;
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
            return targetType->Deserialize(typedObject.Get(), config, value);
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

    static TypePtr CreateType()
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

        return TypePtr(new UniquePtrTypeImpl<T>(typeInfo));
    }
};


} // namespace RTTI
} // namespace NFE
