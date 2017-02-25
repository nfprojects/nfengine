/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of fundamental types for the reflection system.
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

        // extract serialization information
        const Type* elementType = GetType<T>();
        const DynArray<T>& typedObject = *static_cast<const DynArray<T>*>(object);
        const uint32 arraySize = typedObject.Size();

        // sanity checks
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
        UNUSED(outObject);
        UNUSED(config);
        UNUSED(value);
        //TODO
        return false;
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
