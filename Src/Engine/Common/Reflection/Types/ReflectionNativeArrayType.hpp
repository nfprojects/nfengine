/**
 * @file
 * @author Witek902
 * @brief  Definition of NativeArrayType.
 */

#pragma once

#include "ReflectionArrayType.hpp"
#include "../ReflectionTypeResolver.hpp"
#include "../../Config/ConfigInterface.hpp"


namespace NFE {
namespace RTTI {

/**
 * Type information for T[N] types.
 */
class NFCOMMON_API NativeArrayType : public ArrayType
{
    NFE_MAKE_NONCOPYABLE(NativeArrayType)

public:
    NativeArrayType(uint32 arraySize, const Type* underlyingType);

    // get number of array elements
    NFE_FORCE_INLINE uint32 GetArraySize() const { return mArraySize; }
    virtual uint32 GetArraySize(const void* arrayObject) const override final;

    virtual void PrintInfo() const override;

    virtual bool Serialize(const void* object, Common::IConfig& config, Common::ConfigValue& outValue, SerializationContext& context) const override;
    virtual bool SerializeBinary(const void* object, Common::OutputStream* stream, SerializationContext& context) const override;
    virtual bool DeserializeBinary(void* outObject, Common::InputStream& stream, const SerializationContext& context) const override;
    virtual bool Deserialize(void* outObject, const Common::IConfig& config, const Common::ConfigValue& value, const SerializationContext& context) const override;

    virtual bool Compare(const void* objectA, const void* objectB) const override;
    virtual bool Clone(void* destObject, const void* sourceObject) const override;
    virtual bool CanBeMemcopied() const override;

    // access element data
    void* GetElementPointer(void* arrayData, uint32 index) const;
    const void* GetElementPointer(const void* arrayData, uint32 index) const;

protected:
    uint32 mArraySize;              // array size (in elements)
};

/**
 * Generic type creator for T[N] array types.
 */
template<typename T, uint32 N>
class TypeCreator<T[N]>
{
    static_assert(N > 0, "Array size must be non-zero");

public:
    using TypeClass = NativeArrayType;
    using ObjectType = T[N];

    static Type* CreateType()
    {
        return new TypeClass(N, ResolveType<T>());
    }

    static void InitializeType(Type* type)
    {
        const Type* arrayElementType = ResolveType<T>();
        const Common::String typeName = Common::String::Printf("%s[%u]", arrayElementType->GetName().Str(), N);

        TypeInfo typeInfo;
        typeInfo.kind = TypeKind::NativeArray;
        typeInfo.size = sizeof(ObjectType);
        typeInfo.alignment = alignof(ObjectType);
        typeInfo.name = typeName.Str();
        //typeInfo.constructor = GetObjectConstructor<ObjectType>();
        //typeInfo.destructor = GetObjectDestructor<ObjectType>();
        typeInfo.constructor = [] () { return new ObjectType; };
        typeInfo.destructor = [] (void* ptr) { delete BitCast<T*>(ptr); };

        type->Initialize(typeInfo);
    }

    void FinishInitialization(TypeInfo& typeInfo)
    {
        NFE_UNUSED(typeInfo);
    }
};


} // namespace RTTI
} // namespace NFE
