/**
 * @file
 * @author Witek902
 * @brief  Declaration of reflection system's EnumType template class.
 */

#pragma once

#include "../../nfCommon.hpp"
#include "ReflectionType.hpp"
#include "../ReflectionMember.hpp"
#include "../../Config/ConfigValue.hpp"
#include "../../Containers/DynArray.hpp"

namespace NFE {
namespace RTTI {


struct EnumOption
{
    uint64 value;   // we must support 64-bit enums
    Common::StringView name;

    EnumOption(uint64 value, const char* name)
        : value(value)
        , name(name)
    { }
};

using EnumOptions = Common::DynArray<EnumOption>;

/**
 * Helper class used for EnumType initialization.
 */
struct EnumTypeInfo : public TypeInfo
{
    EnumOptions options;
};


/**
 * C++ enum type.
 */
class NFCOMMON_API EnumType final : public Type
{
    NFE_MAKE_NONCOPYABLE(EnumType)

public:
    explicit EnumType(const EnumTypeInfo& info);
    ~EnumType();

    // list all the enum options
    const EnumOptions& GetOptions() const;

    const Common::StringView FindOptionByValue(uint64 value) const;
    bool FindOptionByName(const Common::StringView name, uint64& outValue) const;

    void WriteRawValue(void* object, uint64 rawValue) const;
    uint64 ReadRawValue(const void* object) const;

    void PrintInfo() const override;
    bool Serialize(const void* object, Common::IConfig& config, Common::ConfigValue& outValue, SerializationContext& context) const override;
    bool SerializeBinary(const void* object, Common::OutputStream* stream, SerializationContext& context) const override;
    bool Deserialize(void* outObject, const Common::IConfig& config, const Common::ConfigValue& value, const SerializationContext& context) const override;
    bool DeserializeBinary(void* outObject, Common::InputStream& stream, const SerializationContext& context) const override;
    bool Compare(const void* objectA, const void* objectB) const override;
    bool Clone(void* destObject, const void* sourceObject) const override;

private:
    EnumOptions mOptions;
};


} // namespace RTTI
} // namespace NFE


//////////////////////////////////////////////////////////////////////////


/**
 * Declare an enum type. This must be placed OUTSIDE namespace.
 */
#define NFE_DECLARE_ENUM_TYPE(T)                                                    \
    static_assert(std::is_enum_v<T>, "Given type is not enum type");                \
    namespace NFE { namespace RTTI {                                                \
        template <>                                                                 \
        class TypeCreator<T>                                                        \
        {                                                                           \
        public:                                                                     \
            using TypeClass = EnumType;                                             \
            using TypeInfoClass = EnumTypeInfo;                                     \
            static Type* CreateType()                                               \
            {                                                                       \
                TypeInfoClass typeInfo;                                             \
                typeInfo.kind = TypeKind::Enumeration;                              \
                typeInfo.name = #T;                                                 \
                typeInfo.size = sizeof(T);                                          \
                typeInfo.alignment = alignof(T);                                    \
                typeInfo.constructor = GetObjectConstructor<T>();                   \
                typeInfo.destructor = GetObjectDestructor<T>();                     \
                                                                                    \
                TypeCreator creator;                                                \
                creator.FinishInitialization(typeInfo);                             \
                return new EnumType(typeInfo);                                      \
            }                                                                       \
                                                                                    \
            void FinishInitialization(TypeInfoClass& typeInfo);                     \
        };                                                                          \
    } } /* namespace NFE::RTTI */

