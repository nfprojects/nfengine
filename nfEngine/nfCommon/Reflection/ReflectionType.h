/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's Type class.
 */

#pragma once

#include "ReflectionMember.h"
#include "ReflectionTypeRegistry.h"

#include <vector>


namespace NFE {
namespace RTTI {



/**
 * Type information.
 * Can be primitive type as well as complex class.
 */
class NFCOMMON_API Type
{
public:
    using Members = std::vector<Member>;

    NFE_INLINE Type()
        : mSize(0)
        , mAlignment(0)
        , mParent(nullptr)
    { }

    NFE_INLINE const std::string& GetName() const { return mName; }
    NFE_INLINE size_t GetSize() const { return mSize; }
    NFE_INLINE size_t GetAlignment() const { return mAlignment; }
    NFE_INLINE const Type* GetParent() const { return mParent; }
    NFE_INLINE const Members& GetMembers() const { return mMembers; }

    /**
     * Check if this type is derived from the other type.
     */
    bool IsDerivedFrom(const Type* type) const;

    static bool DefineInheritance(Type* parent, Type* child);

public: // TODO
    std::string mName;

    size_t mSize;
    size_t mAlignment;

    Type* mParent;
    std::vector<Type*> mChildTypes;
    Members mMembers;
};

/**
 * Template class used to generate Type class instances.
 * This template will be specialized in NFE_DECLARE_TYPE macro.
 */
template <typename T>
class TypeCreator
{
public:
    static const Type* GetType()
    {
        // if you get this error, this means you didn't used NFE_DECLARE_TYPE(T)
        static_assert(false, "Type is not declared");
        return nullptr;
    }

private:
    static const Type* GetTypeIntenal()
    {
        return nullptr;
    }
};

} // namespace RTTI
} // namespace NFE
