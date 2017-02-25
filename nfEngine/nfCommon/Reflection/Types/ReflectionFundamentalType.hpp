/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's Type class.
 */

#pragma once

#include "../../nfCommon.hpp"
#include "ReflectionType.hpp"
#include "../ReflectionMember.hpp"
#include "../../Config/ConfigValue.hpp"

// TODO replace with DynArray and String
#include <vector>
#include <string>


namespace NFE {
namespace RTTI {

/**
 * Fundamental C++ types (bool, integers, floating point).
 */
template<typename T>
class FundamentalType : public Type
{
    NFE_MAKE_NONCOPYABLE(FundamentalType);

public:
    FundamentalType(const TypeInfo& info)
        : Type(info)
    { }

    FundamentalType(FundamentalType&&) = default;
    FundamentalType& operator=(FundamentalType&&) = default;

    /**
     * Write an object of this type to a config value.
     *
     * @param   object              Pointer to a source object of "this" type.
     * @param   config,outValue     Target config value to write.
     * @return  True on success.
     */
    bool Serialize(const void* object, Common::Config& config, Common::ConfigValue& outValue) const override
    {
        // no need to access Config object itself, all the data for fundamental types is contained in ConfigValue
        UNUSED(config);

        const T* typedObject = static_cast<const T*>(object);
        outValue = Common::ConfigValue(*typedObject);
        return true;
    }

    /**
     * Read an object of this type from a config value.
     *
     * @param   outObject       Pointer to a target object of "this" type.
     * @param   config,value    Source config value.
     * @return  True on success.
     */
    bool Deserialize(void* outObject, const Common::Config& config, const Common::ConfigValue& value) const override
    {
        UNUSED(config);

        // TODO type casting (with check if there is no overflow)
        if (value.Is<T>())
        {
            T* typedObject = static_cast<T*>(outObject);
            *typedObject = value.Get<T>();
            return true;
        }

        return false;
    }
};


} // namespace RTTI
} // namespace NFE


//////////////////////////////////////////////////////////////////////////


/**
 * Declare a type. This must be placed OUTSIDE namespace.
 */
#define NFE_DECLARE_FUNDAMENTAL_TYPE(T)                                         \
    namespace NFE { namespace RTTI {                                            \
        template <>                                                             \
        class TypeCreator<T>                                                    \
        {                                                                       \
        public:                                                                 \
            using TypeClass = FundamentalType<T>;                               \
            using TypeInfoClass = TypeInfo;                                     \
            static TypePtr CreateType()                                         \
            {                                                                   \
                TypeInfo typeInfo;                                              \
                typeInfo.kind = TypeKind::Fundamental;                          \
                typeInfo.name = #T;                                             \
                typeInfo.size = sizeof(T);                                      \
                typeInfo.alignment = alignof(T);                                \
                TypePtr type(new FundamentalType<T>(typeInfo));                 \
                return type;                                                    \
            }                                                                   \
            void FinishInitialization(TypeInfoClass& typeInfo)                  \
            {                                                                   \
                UNUSED(typeInfo);                                               \
            }                                                                   \
        };                                                                      \
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
