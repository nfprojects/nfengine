/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's Type class.
 */

#pragma once

#include "ReflectionMember.h"

#include <vector>


namespace NFE {
namespace RTTI {

/**
 * Type information.
 * Can be primitive type as well as complex class.
 */
class Type
{
public:
    NFE_INLINE Type()
        : mSize(0)
        , mAlignment(0)
        , mParent(nullptr)
    { }

    NFE_INLINE const std::string& GetName() const { return mName; }
    NFE_INLINE size_t GetSize() const { return mSize; }
    NFE_INLINE size_t GetAlignment() const { return mAlignment; }
    NFE_INLINE const Type* GetParent() const { return mParent; }

    static bool DefineInheritance(Type* parent, Type* child);

public: // TODO
    std::string mName;

    size_t mSize;
    size_t mAlignment;

    Type* mParent;
    std::vector<Type*> mChildTypes;
    std::vector<Member> mMembers;
};

/**
 * Template class used to generate Type class instances via template specialization.
 */
template <typename T>
class TypeCreator final
{
public:
    TypeCreator()
    {
        Type* type = GetInternal();
        Initialize(type);
    }

    // TODO const
    static Type* Get()
    {
        return GetInternal();
    }

    static const T* GetNullPtr()
    {
        return nullptr;
    }

    bool Initialize(Type* type);

private:
    static Type* GetInternal()
    {
        static Type type;
        return &type;
    }
};

} // namespace RTTI
} // namespace NFE
