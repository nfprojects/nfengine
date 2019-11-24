/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's FundamentalType template class.
 */

#pragma once

#include "../../nfCommon.hpp"
#include "ReflectionType.hpp"
#include "../ReflectionMember.hpp"
#include "../../Config/ConfigValue.hpp"


namespace NFE {
namespace RTTI {

/**
 * Fundamental C++ types (bool, integers, floating point).
 */
template<typename T>
class FundamentalType final : public Type
{
    NFE_MAKE_NONCOPYABLE(FundamentalType)

public:
    FundamentalType(const TypeInfo& info)
        : Type(info)
    { }

    bool Serialize(const void* object, Common::Config& config, Common::ConfigValue& outValue) const override
    {
        // no need to access Config object itself, all the data for fundamental types is contained in ConfigValue
        NFE_UNUSED(config);

        const T* typedObject = static_cast<const T*>(object);
        outValue = Common::ConfigValue(*typedObject);
        return true;
    }

    bool Deserialize(void* outObject, const Common::Config& config, const Common::ConfigValue& value) const override
    {
        NFE_UNUSED(config);

        // TODO type casting (with check if there is no overflow)
        if (value.Is<T>())
        {
            T* typedObject = static_cast<T*>(outObject);
            *typedObject = value.Get<T>();
            return true;
        }

        return false;
    }

    bool Compare(const void* objectA, const void* objectB) const override
    {
        const T* typedObjectA = static_cast<const T*>(objectA);
        const T* typedObjectB = static_cast<const T*>(objectB);
        return (*typedObjectA) == (*typedObjectB);
    }
};


} // namespace RTTI
} // namespace NFE


//////////////////////////////////////////////////////////////////////////


/**
 * Declare a type. This must be placed OUTSIDE namespace.
 */
#define NFE_DECLARE_FUNDAMENTAL_TYPE(T)                                                 \
    static_assert(std::is_fundamental_v<T>, "Given type is not fundamental");           \
    namespace NFE { namespace RTTI {                                                    \
        template <>                                                                     \
        class TypeCreator<T>                                                            \
        {                                                                               \
        public:                                                                         \
            using TypeClass = FundamentalType<T>;                                       \
            using TypeInfoClass = TypeInfo;                                             \
            static Type* CreateType()                                                   \
            {                                                                           \
                TypeInfo typeInfo;                                                      \
                typeInfo.kind = TypeKind::Fundamental;                                  \
                typeInfo.name = #T;                                                     \
                typeInfo.size = sizeof(T);                                              \
                typeInfo.alignment = alignof(T);                                        \
                typeInfo.constructor = []() { return new T(); };                        \
                typeInfo.arrayConstructor = [](uint32 num) { return new T[num](); };    \
                return new FundamentalType<T>(typeInfo);                                \
            }                                                                           \
        };                                                                              \
    } } /* namespace NFE::RTTI */


//////////////////////////////////////////////////////////////////////////


NFE_DECLARE_FUNDAMENTAL_TYPE(bool)
NFE_DECLARE_FUNDAMENTAL_TYPE(float)
NFE_DECLARE_FUNDAMENTAL_TYPE(double)

NFE_DECLARE_FUNDAMENTAL_TYPE(NFE::uint8)
NFE_DECLARE_FUNDAMENTAL_TYPE(NFE::uint16)
NFE_DECLARE_FUNDAMENTAL_TYPE(NFE::uint32)
NFE_DECLARE_FUNDAMENTAL_TYPE(NFE::uint64)

NFE_DECLARE_FUNDAMENTAL_TYPE(NFE::int8)
NFE_DECLARE_FUNDAMENTAL_TYPE(NFE::int16)
NFE_DECLARE_FUNDAMENTAL_TYPE(NFE::int32)
NFE_DECLARE_FUNDAMENTAL_TYPE(NFE::int64)
