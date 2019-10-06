/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's EnumType template class.
 */

#pragma once

#include "../../nfCommon.hpp"
#include "ReflectionType.hpp"
#include "../ReflectionMember.hpp"
#include "../../Config/ConfigValue.hpp"
#include "../../Containers/String.hpp"
#include "../../Containers/DynArray.hpp"

namespace NFE {
namespace RTTI {


struct EnumOption
{
    uint64 value;   // we must support 64-bit enums
    const char* name;

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

    const char* FindOptionByValue(uint64 value) const;
    bool WriteValue(void* object, uint32 enumOptionIndex) const;
    bool ReadValue(const void* object, uint32& outEnumOptionIndex) const;

    void PrintInfo() const override;
    bool Serialize(const void* object, Common::Config& config, Common::ConfigValue& outValue) const override;
    bool Deserialize(void* outObject, const Common::Config& config, const Common::ConfigValue& value) const override;
    bool Compare(const void* objectA, const void* objectB) const override;

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
            static TypePtr CreateType()                                             \
            {                                                                       \
                TypeInfoClass typeInfo;                                             \
                typeInfo.kind = TypeKind::Enumeration;                              \
                typeInfo.name = #T;                                                 \
                typeInfo.size = sizeof(T);                                          \
                typeInfo.alignment = alignof(T);                                    \
                typeInfo.constructor = []() { return new T; };                      \
                typeInfo.arrayConstructor = [](uint32 num) { return new T[num]; };  \
                                                                                    \
                TypeCreator creator;                                                \
                creator.FinishInitialization(typeInfo);                             \
                return TypePtr(new EnumType(typeInfo));                             \
            }                                                                       \
                                                                                    \
            void FinishInitialization(TypeInfoClass& typeInfo);                     \
        };                                                                          \
    } } /* namespace NFE::RTTI */

