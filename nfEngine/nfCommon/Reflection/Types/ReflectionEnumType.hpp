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


/**
 * Helper class used for EnumType initialization.
 */
struct EnumTypeInfo : public TypeInfo
{
    Common::DynArray<EnumOption> options;
};


/**
 * C++ enum type.
 */
class EnumType : public Type
{
    NFE_MAKE_NONCOPYABLE(EnumType)

public:
    EnumType(const EnumTypeInfo& info);
    EnumType(EnumType&&) = default;
    EnumType& operator=(EnumType&&) = default;

    bool Serialize(const void* object, Common::Config& config, Common::ConfigValue& outValue) const override;
    bool Deserialize(void* outObject, const Common::Config& config, const Common::ConfigValue& value) const override;

private:
    Common::DynArray<EnumOption> mOptions;
};


} // namespace RTTI
} // namespace NFE


//////////////////////////////////////////////////////////////////////////


/**
 * Declare an enum type. This must be placed OUTSIDE namespace.
 */
#define NFE_DECLARE_ENUM_TYPE(T)                                                    \
    static_assert(std::is_enum<T>::value, "Given type is not enum type");           \
    namespace NFE { namespace RTTI {                                                \
        template <>                                                                 \
        class TypeCreator<T>                                                        \
        {                                                                           \
        public:                                                                     \
            using TypeClass = EnumType;                                             \
            using TypeInfoClass = EnumTypeInfo;                                     \
            static TypePtr CreateType()                                             \
            {                                                                       \
                EnumTypeInfo typeInfo;                                              \
                typeInfo.kind = TypeKind::Enumeration;                              \
                typeInfo.name = #T;                                                 \
                typeInfo.size = sizeof(T);                                          \
                typeInfo.alignment = alignof(T);                                    \
                typeInfo.constructor = []() { return new T; };                      \
                typeInfo.arrayConstructor = [](uint32 num) { return new T[num]; };  \
                return TypePtr(new TypeClass(typeInfo));                            \
            }                                                                       \
                                                                                    \
            void FinishInitialization(TypeInfoClass& typeInfo);                     \
        };                                                                          \
    } } /* namespace NFE::RTTI */

