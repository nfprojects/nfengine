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
 * Type information for UniquePtr<T> types.
 */
template<typename T>
class UniquePtrType : public Type
{
    NFE_MAKE_NONCOPYABLE(UniquePtrType)

public:
    UniquePtrType(const TypeInfo& info)
        : Type(info)
    {
        mPointedType = GetType<T>();
        NFE_ASSERT(mPointedType, "Invalid poined type");
    }

    bool Serialize(const void* object, Common::Config& config, Common::ConfigValue& outValue) const override
    {
        NFE_ASSERT(object, "Trying to serialize nullptr");
        const std::unique_ptr<T>& typedObject = *static_cast<const std::unique_ptr<T>*>(object);

        if (typedObject)
        {
            mPointedType->Serialize(typedObject.get(), config, outValue);
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
        std::unique_ptr<T>& typedObject = *static_cast<std::unique_ptr<T>*>(outObject);

        if (value.Is<int32>()) // nullptr
        {
            if (value.Get<int32>() != 0)
            {
                LOG_WARNING("Expected zero");
            }

            typedObject.reset();
            return true;
        }
        else if (value.IsObject()) // valid object
        {
            const char* typeName = nullptr;
            const auto configIteratorCallback = [&typeName](const char* key, const Common::ConfigValue& value)
            {
                // marker found
                if (strcmp(key, ClassType::TYPE_MARKER) == 0)
                {
                    if (!value.IsString())
                    {
                        LOG_ERROR("Marker type found - string expected");
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
                targetType = TypeRegistry::GetInstance().GetExistingTypeByName(typeName);
                if (!targetType)
                {
                    LOG_ERROR("Type not found: '%s'", typeName);
                    return false;
                }

                if (!targetType->IsA(mPointedType))
                {
                    LOG_ERROR("Target type '%s' is not related with pointed type '%s'", typeName, mPointedType->GetName());
                    return false;
                }
            }
            else
            {
                if (mPointedType->GetKind() != TypeKind::AbstractClass)
                {
                    LOG_WARNING("Type marker not found - using pointed type as a reference");
                    targetType = mPointedType;
                }
                else // pointed type is abstract class
                {
                    LOG_ERROR("Type marker not found - cannot resolve target type");
                    return false;
                }
            }

            if (!targetType->IsConstructible())
            {
                LOG_ERROR("Target type '%s' is not constructible", typeName);
                return false;
            }

            // construct the object & assign to the smart pointer
            typedObject.reset(targetType->CreateObject<T>());
            if (!typedObject)
            {
                LOG_ERROR("Failed to create object");
                return false;
            }

            // deserialize the object
            return targetType->Deserialize(typedObject.get(), config, value);
        }

        LOG_ERROR("Expected zero (nullptr) or an object");
        return false;
    }

private:
    const Type* mPointedType;
};


/**
 * Generic type creator for UniquePtr<T> types.
 */
template<typename T>
class TypeCreator<std::unique_ptr<T>> // TODO UniquePtr
{
public:
    using TypeClass = UniquePtrType<T>;

    static TypePtr CreateType()
    {
        const Type* templateArgumentType = GetType<T>();

        std::string typeName = std::string("std::unique_ptr<") + templateArgumentType->GetName() + '>';

        TypeInfo typeInfo;
        typeInfo.kind = TypeKind::UniquePtr;
        typeInfo.size = sizeof(std::unique_ptr<T>);
        typeInfo.alignment = alignof(std::unique_ptr<T>);
        typeInfo.name = typeName.c_str();
        typeInfo.constructor = []() { return new std::unique_ptr<T>; };
        typeInfo.arrayConstructor = [](uint32 num) { return new std::unique_ptr<T>[num]; };

        return TypePtr(new UniquePtrType<T>(typeInfo));
    }
};


} // namespace RTTI
} // namespace NFE
