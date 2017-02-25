/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of fundamental types for the reflection system.
 */

#pragma once

#include "ReflectionType.hpp"
#include "ReflectionTypeSerializer.hpp"
#include "../Containers/DynArray.hpp"
#include "../Config/Config.hpp"
#include "../Config/ConfigValue.hpp"
#include "../Utils/InputStream.hpp"
#include "../Utils/OutputStream.hpp"


namespace NFE {
namespace RTTI {

/**
 * Serializer for DynArray<> type.
 */
template<typename T>
class DynArraySerializer : public ITypeSerializer
{
public:
    bool Serialize(const void* object, Common::OutputStream& stream) const override
    {
        UNUSED(object);
        UNUSED(stream);
        //TODO
        return false;
    }

    bool Deserialize(void* outObject, Common::InputStream& stream) const override
    {
        UNUSED(outObject);
        UNUSED(stream);
        //TODO
        return false;
    }

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
 * Generic type creator for DynArray<> types.
 */
template<typename T>
class TypeCreator<Common::DynArray<T>>
{
public:
    static const Type* GetType()
    {
        return GetTypeIntenal();
    }

private:
    static const Type* GetTypeIntenal()
    {
        const Type* templateArgumentType = ::GetType<T>();

        std::string typeName = "NFE::Common::DynArray";
        typeName += "<";
        typeName += templateArgumentType->GetName();
        typeName += ">";

        static Type* type = Init(typeName.c_str());
        return type;
    }

    static Type* Init(const char* typeName)
    {
        Type* type = TypeRegistry::GetInstance().GetType(typeName);

        TypeInfo typeInfo;
        typeInfo.kind = TypeKind::SimpleClass;
        typeInfo.name = typeName;
        typeInfo.size = sizeof(T);
        typeInfo.alignment = alignof(T);
        typeInfo.serializer.reset(new DynArraySerializer<T>);
        type->FinishInitialization(std::move(typeInfo));

        return type;
    }
};

} // namespace RTTI
} // namespace NFE
