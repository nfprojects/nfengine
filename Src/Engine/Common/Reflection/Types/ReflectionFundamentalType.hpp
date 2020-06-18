/**
 * @file
 * @author Witek902
 * @brief  Declaration of reflection system's FundamentalType template class.
 */

#pragma once

#include "../../nfCommon.hpp"
#include "ReflectionType.hpp"
#include "../../Config/ConfigValue.hpp"
#include "../../Utils/LanguageUtils.hpp"


namespace NFE {
namespace RTTI {


// base class for all fundamental types
class NFCOMMON_API FundamentalType : public Type
{
public:
    FundamentalType();

    virtual bool SerializeBinary(const void* object, Common::OutputStream* stream, SerializationContext& context) const override final;
    virtual bool DeserializeBinary(void* outObject, Common::InputStream& stream, const SerializationContext& context) const override final;
    virtual bool CanBeMemcopied() const override final { return true; }
};


/**
 * Fundamental C++ types (bool, integers, floating point).
 */
template<typename T>
class FundamentalTypeImpl final : public FundamentalType
{
    NFE_MAKE_NONCOPYABLE(FundamentalTypeImpl)

public:
    FundamentalTypeImpl() = default;

    virtual bool Serialize(const void* object, Common::IConfig& config, Common::ConfigValue& outValue, SerializationContext& context) const override
    {
        // no need to access Config object itself, all the data for fundamental types is contained in ConfigValue
        NFE_UNUSED(config);
        NFE_UNUSED(context);

        const T* typedObject = static_cast<const T*>(object);
        outValue = Common::ConfigValue(*typedObject);
        return true;
    }

    virtual bool Deserialize(void* outObject, const Common::IConfig& config, const Common::ConfigValue& value, const SerializationContext& context) const override
    {
        NFE_UNUSED(config);
        NFE_UNUSED(context);

        // TODO type casting (with check if there is no overflow)
        // TODO handle type mismatch
        if (value.Is<T>())
        {
            T* typedObject = static_cast<T*>(outObject);
            *typedObject = value.Get<T>();
            return true;
        }

        return false;
    }

    virtual bool Compare(const void* objectA, const void* objectB) const override
    {
        const T* typedObjectA = static_cast<const T*>(objectA);
        const T* typedObjectB = static_cast<const T*>(objectB);
        return (*typedObjectA) == (*typedObjectB);
    }

    virtual bool Clone(void* destObject, const void* sourceObject) const override
    {
        *static_cast<T*>(destObject) = *static_cast<const T*>(sourceObject);
        return true;
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
            using TypeClass = FundamentalTypeImpl<T>;                                   \
            using TypeInfoClass = TypeInfo;                                             \
            static Type* CreateType()                                                   \
            {                                                                           \
                return new TypeClass;                                                   \
            }                                                                           \
            static void InitializeType(Type* type)                                      \
            {                                                                           \
                TypeInfo typeInfo;                                                      \
                typeInfo.kind = TypeKind::Fundamental;                                  \
                typeInfo.name = #T;                                                     \
                typeInfo.size = sizeof(T);                                              \
                typeInfo.alignment = alignof(T);                                        \
                typeInfo.constructor = []() { return new T(); };                        \
                typeInfo.destructor = [] (void* ptr) { delete BitCast<T*>(ptr); };      \
                type->Initialize(typeInfo);                                             \
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
