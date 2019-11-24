/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of DynArrayPtr.
 */

#pragma once

#include "ReflectionType.hpp"
#include "../../Containers/DynArray.hpp"
#include "../../Config/Config.hpp"
#include "../../Config/ConfigValue.hpp"


namespace NFE {
namespace RTTI {

/**
 * Type information for DynArray<T> types.
 */
class NFCOMMON_API DynArrayType : public Type
{
    NFE_MAKE_NONCOPYABLE(DynArrayType)

public:
    NFE_FORCE_INLINE DynArrayType(const TypeInfo& info) : Type(info) { }

    // get number of array elements
    uint32 GetArraySize(const void* arrayObject) const;

    // get type of the array element
    virtual const Type* GetUnderlyingType() const = 0;

    virtual void PrintInfo() const override;

    bool Compare(const void* objectA, const void* objectB) const override;

    // access element data
    virtual void* GetElementPointer(void* arrayObject, uint32 index) const = 0;
    virtual const void* GetElementPointer(const void* arrayObject, uint32 index) const = 0;
};

/**
 * Specialized type information for DynArray<T> types.
 */
template<typename T>
class DynArrayTypeImpl final : public DynArrayType
{
    NFE_MAKE_NONCOPYABLE(DynArrayTypeImpl)

public:
    NFE_FORCE_INLINE DynArrayTypeImpl(const TypeInfo& info) : DynArrayType(info) { }

    virtual const Type* GetUnderlyingType() const override
    {
        return GetType<T>();
    }

    virtual void* GetElementPointer(void* arrayData, uint32 index) const override
    {
        Common::DynArray<T>& typedObject = *static_cast<Common::DynArray<T>*>(arrayData);
        return &typedObject[index];
    }

    virtual const void* GetElementPointer(const void* arrayData, uint32 index) const override
    {
        const Common::DynArray<T>& typedObject = *static_cast<const Common::DynArray<T>*>(arrayData);
        return &typedObject[index];
    }

    bool Serialize(const void* object, Common::Config& config, Common::ConfigValue& outValue) const override
    {
        using namespace Common;

        const Type* elementType = GetType<T>();
        const DynArray<T>& typedObject = *static_cast<const DynArray<T>*>(object);
        const uint32 arraySize = typedObject.Size();

        NFE_ASSERT(elementType, "Invalid DynArray element type");
        NFE_ASSERT(object, "Trying to serialize nullptr");

        // serialize array elements
        ConfigArray configArray;
        for (uint32 i = 0; i < arraySize; ++i)
        {
            ConfigValue arrayElementValue;
            const T& arrayElement = typedObject[i];
            if (!elementType->Serialize(&arrayElement, config, arrayElementValue))
            {
                NFE_LOG_ERROR("Failed to serialize DynArray element (index %u/%u)", i, arraySize);
                return false;
            }
            config.AddValue(configArray, arrayElementValue);
        }

        // success
        outValue = configArray;
        return true;
    }

    bool Deserialize(void* outObject, const Common::Config& config, const Common::ConfigValue& value) const override
    {
        using namespace Common;

        const Type* elementType = GetType<T>();
        DynArray<T>& typedObject = *static_cast<DynArray<T>*>(outObject);

        NFE_ASSERT(elementType, "Invalid native array element type");
        NFE_ASSERT(outObject, "Trying to deserialize to nullptr");

        if (!value.IsArray())
        {
            NFE_LOG_ERROR("Expected array type");
            return false;
        }

        uint32 numDeserializedArrayElements = 0;
        auto arrayIteratorCallback = [&](int index, const ConfigValue& arrayElement)
        {
            // TODO deserialize directly to the DynArray
            T temp;
            if (!elementType->Deserialize(&temp, config, arrayElement))
            {
                NFE_LOG_ERROR("Failed to parse native array element at index %i", index);
                return false;
            }

            typedObject.PushBack(std::move(temp));

            numDeserializedArrayElements++;
            return true;
        };

        config.IterateArray(arrayIteratorCallback, value.GetArray());
        return true;
    }
};


/**
 * Generic type creator for DynArray<T> types.
 */
template<typename T>
class TypeCreator<Common::DynArray<T>>
{
public:
    using TypeClass = DynArrayTypeImpl<T>;
    using TypeInfoClass = TypeInfo;

    static Type* CreateType()
    {
        const Type* templateArgumentType = GetType<T>();

        const Common::String typeName = Common::String("NFE::Common::DynArray<") + templateArgumentType->GetName() + '>';

        TypeInfo typeInfo;
        typeInfo.kind = TypeKind::DynArray;
        typeInfo.size = sizeof(Common::DynArray<T>);
        typeInfo.alignment = alignof(Common::DynArray<T>);
        typeInfo.name = typeName.Str();
        typeInfo.constructor = []() { return new Common::DynArray<T>; };
        typeInfo.arrayConstructor = [](uint32 num) { return new Common::DynArray<T>[num]; };

        return new DynArrayTypeImpl<T>(typeInfo);
    }
};


} // namespace RTTI
} // namespace NFE
