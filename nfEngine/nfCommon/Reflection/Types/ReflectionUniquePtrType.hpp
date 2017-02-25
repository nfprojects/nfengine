/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of UniquePtrType.
 */

#pragma once

#include "ReflectionType.hpp"
#include "../../Config/Config.hpp"
#include "../../Config/ConfigValue.hpp"

#include <memory>


namespace NFE {
namespace RTTI {

/**
 * Type information for DynArray<T> types.
 */
template<typename T>
class UniquePtrType : public Type
{
    NFE_MAKE_NONCOPYABLE(UniquePtrType)

public:
    UniquePtrType(const TypeInfo& info)
        : Type(info)
    { }

    bool Serialize(const void* object, Common::Config& config, Common::ConfigValue& outValue) const override
    {
        NFE_ASSERT(object, "Trying to serialize nullptr");

        using namespace Common;

        // extract serialization information
        const Type* pointedType = GetType<T>();
        const std::unique_ptr<T>& typedObject = *static_cast<const std::unique_ptr<T>*>(object);

        if (typedObject)
        {
            pointedType->Serialize(typedObject.get(), config, outValue);
        }
        else // null pointer
        {
            outValue = Common::ConfigValue(0);
        }

        return true;
    }

    bool Deserialize(void* outObject, const Common::Config& config, const Common::ConfigValue& value) const override
    {
        UNUSED(outObject);
        UNUSED(config);
        UNUSED(value);
        // TODO construct object
        return false;
    }
};


/**
 * Generic type creator for DynArray<T> types.
 */
template<typename T>
class TypeCreator<std::unique_ptr<T>> // TODO UniquePtr
{
public:
    using TypeClass = UniquePtrType<T>;
    using TypeInfoClass = TypeInfo;

    static TypePtr CreateType()
    {
        const Type* templateArgumentType = GetType<T>();

        std::string typeName = std::string("std::unique_ptr<") + templateArgumentType->GetName() + '>';

        TypeInfo typeInfo;
        typeInfo.kind = TypeKind::UniquePtr;
        typeInfo.size = sizeof(std::unique_ptr<T>);
        typeInfo.alignment = alignof(std::unique_ptr<T>);
        typeInfo.name = typeName.c_str();

        return TypePtr(new UniquePtrType<T>(typeInfo));
    }
};


} // namespace RTTI
} // namespace NFE
