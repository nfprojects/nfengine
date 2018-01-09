/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of NativeArrayType.
 */

#pragma once

#include "ReflectionType.hpp"
#include "../../Config/Config.hpp"
#include "../../Config/ConfigValue.hpp"


namespace NFE {
namespace RTTI {

/**
 * Type information for T[N] types.
 */
template<typename T>
class NativeArrayType : public Type
{
    NFE_MAKE_NONCOPYABLE(NativeArrayType)

public:
    NativeArrayType(const TypeInfo& info, uint32 arraySize)
        : Type(info)
        , mArraySize(arraySize)
    { }

    void PrintInfo() const override
    {
        Type::PrintInfo();
        NFE_LOG_DEBUG("  array size = %u", mArraySize);
    }

    bool Serialize(const void* object, Common::Config& config, Common::ConfigValue& outValue) const override
    {
        using namespace Common;

        // extract serialization information
        const Type* elementType = GetType<T>();
        const T* typedArray = static_cast<const T*>(object);

        // sanity checks
        NFE_ASSERT(elementType, "Invalid native array element type");
        NFE_ASSERT(object, "Trying to serialize nullptr");

        // serialize array elements
        ConfigArray configArray;
        for (uint32 i = 0; i < mArraySize; ++i)
        {
            ConfigValue arrayElementValue;
            const T& arrayElement = typedArray[i];
            if (!elementType->Serialize(&arrayElement, config, arrayElementValue))
            {
                NFE_LOG_ERROR("Failed to serialize native array element (index %u/%u)", i, mArraySize);
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

        // extract serialization information
        const Type* elementType = GetType<T>();
        T* typedArray = static_cast<T*>(outObject);

        // sanity checks
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
            if (static_cast<uint32>(index) >= mArraySize)
            {
                NFE_LOG_WARNING("Deserialized native array object has too many elements (%u expected). "
                            "Elements will be dropped", mArraySize);
                return false;
            }

            if (!elementType->Deserialize(typedArray + index, config, arrayElement))
            {
                NFE_LOG_ERROR("Failed to parse native array element at index %i", index);
                return false;
            }

            numDeserializedArrayElements++;
            return true;
        };

        config.IterateArray(arrayIteratorCallback, value.GetArray());

        if (numDeserializedArrayElements != mArraySize)
        {
            NFE_LOG_WARNING("Deserialized native array object has too few array elements (%u found, %u expected).",
                        numDeserializedArrayElements, mArraySize);
            // TODO initialize missing elements with default values (run default constructor)
        }

        return true;
    }

private:
    // array size (in elements)
    uint32 mArraySize;
};


/**
 * Generic type creator for T[N] array types.
 */
template<typename T, uint32 N>
class TypeCreator<T[N]>
{
    static_assert(N > 0, "Array size must be non-zero");

public:
    using TypeClass = DynArrayType<T>;

    static TypePtr CreateType()
    {
        const Type* templateArgumentType = GetType<T>();
        std::string typeName = std::string(templateArgumentType->GetName()) + "[]";

        TypeInfo typeInfo;
        typeInfo.kind = TypeKind::NativeArray;
        typeInfo.size = sizeof(T[N]);
        typeInfo.alignment = alignof(T[N]);
        typeInfo.name = typeName.c_str();
        typeInfo.constructor = []() { return new T[N]; };
        //typeInfo.arrayConstructor = [](uint32 num) { return new T[N][num]; };

        return TypePtr(new NativeArrayType<T>(typeInfo, N));
    }

    void FinishInitialization(TypeInfo& typeInfo)
    {
        NFE_UNUSED(typeInfo);
    }
};


} // namespace RTTI
} // namespace NFE
