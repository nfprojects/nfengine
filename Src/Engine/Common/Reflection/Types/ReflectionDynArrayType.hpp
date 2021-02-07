/**
 * @file
 * @author Witek902
 * @brief  Definition of DynArrayPtr.
 */

#pragma once

#include "ReflectionResizableArrayType.hpp"
#include "../ReflectionTypeResolver.hpp"
#include "../../Containers/DynArray.hpp"


namespace NFE {
namespace RTTI {

/**
 * Type information for DynArray<T> types.
 */
class NFCOMMON_API DynArrayType final : public ResizableArrayType
{
    NFE_MAKE_NONCOPYABLE(DynArrayType)

public:
    NFE_FORCE_INLINE DynArrayType(const Type* underlyingType)
        : ResizableArrayType(underlyingType)
    { }

    static const Common::String BuildTypeName(const Type* underlyingType);

    bool ReserveArray(void* arrayObject, uint32 targetCapacity) const;

    // ResizableArrayType interface
    virtual uint32 GetArraySize(const void* arrayObject) const override final;
    virtual bool ResizeArray(void* arrayObject, uint32 targetSize) const override final;
    virtual void* GetElementPointer(void* arrayObject, uint32 index) const override final;
    virtual const void* GetElementPointer(const void* arrayObject, uint32 index) const override final;

    // Type interface implementation
    virtual void PrintInfo() const override final;
};


/**
 * Generic type creator for DynArray<T> types.
 */
template<typename T>
class TypeCreator<Common::DynArray<T>>
{
public:
    using TypeClass = DynArrayType;
    using TypeInfoClass = TypeInfo;
    using ObjectType = Common::DynArray<T>;

    static Type* CreateType()
    {
        const Type* underlyingType = ResolveType<T>();
        return new DynArrayType(underlyingType);
    }

    static void InitializeType(Type* type)
    {
        const Type* underlyingType = ResolveType<T>();

        const Common::String typeName = DynArrayType::BuildTypeName(underlyingType);

        TypeInfo typeInfo;
        typeInfo.kind = TypeKind::Array;
        typeInfo.typeNameID = TypeNameID::Array;
        typeInfo.size = sizeof(ObjectType);
        typeInfo.alignment = alignof(ObjectType);
        typeInfo.name = typeName.Str();
        typeInfo.constructor = GetObjectConstructor<ObjectType>();
        typeInfo.destructor = GetObjectDestructor<ObjectType>();

        type->Initialize(typeInfo);
    }
};


} // namespace RTTI
} // namespace NFE
