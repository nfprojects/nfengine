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
template<typename T>
class DynArrayType : public Type
{
    NFE_MAKE_NONCOPYABLE(DynArrayType);

public:
    DynArrayType(const TypeInfo& info)
        : Type(info)
    { }

    DynArrayType(DynArrayType&&) = default;
    DynArrayType& operator=(DynArrayType&&) = default;

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
                LOG_ERROR("Failed to serialize DynArray element (index %u/%u)", i, arraySize);
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
            LOG_ERROR("Expected array type");
            return false;
        }

        uint32 numDeserializedArrayElements = 0;
        auto arrayIteratorCallback = [&](int index, const ConfigValue& arrayElement)
        {
            // TODO deserialize directly to the DynArray
            T temp;
            if (!elementType->Deserialize(&temp, config, arrayElement))
            {
                LOG_ERROR("Failed to parse native array element at index %i", index);
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
    using TypeClass = DynArrayType<T>;
    using TypeInfoClass = TypeInfo;

    static TypePtr CreateType()
    {
        const Type* templateArgumentType = GetType<T>();

        std::string typeName = std::string("NFE::Common::DynArray<") + templateArgumentType->GetName() + '>';

        TypeInfo typeInfo;
        typeInfo.kind = TypeKind::DynArray;
        typeInfo.size = sizeof(Common::DynArray<T>);
        typeInfo.alignment = alignof(Common::DynArray<T>);
        typeInfo.name = typeName.c_str();

        return TypePtr(new DynArrayType<T>(typeInfo));
    }

    void FinishInitialization(TypeInfo& typeInfo)
    {
        UNUSED(typeInfo);
    }
};


} // namespace RTTI
} // namespace NFE
