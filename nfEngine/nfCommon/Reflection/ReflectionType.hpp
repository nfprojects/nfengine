/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's Type class.
 */

#pragma once

#include "../nfCommon.hpp"
#include "ReflectionMember.hpp"
#include "ReflectionTypeRegistry.hpp"

#include <vector>


namespace NFE {
namespace RTTI {

/**
 * Instead of filling Type class directly, this class is used as a helper.
 */
struct TypeInfo
{
    std::string name;

    size_t size;
    size_t alignment;

    const Type* parent;
    std::vector<const Type*> childTypes;
    std::vector<Member> members;
};

/**
 * Type information.
 * Can be primitive type as well as complex class.
 */
class NFCOMMON_API Type
{
public:
    template <typename T>
    friend class TypeCreator;

    using Members = std::vector<Member>;

    NFE_INLINE Type()
        : mSize(0)
        , mAlignment(0)
        , mParent(nullptr)
    { }

    Type(const Type&) = default;
    Type(Type&&) = default;
    Type& operator=(const Type&) = default;
    Type& operator=(Type&&) = default;

    NFE_INLINE const std::string& GetName() const { return mName; }
    NFE_INLINE size_t GetSize() const { return mSize; }
    NFE_INLINE size_t GetAlignment() const { return mAlignment; }
    NFE_INLINE const Type* GetParent() const { return mParent; }
    NFE_INLINE const Members& GetMembers() const { return mMembers; }

    /**
     * Check if this type is derived from the other type.
     */
    bool IsDerivedFrom(const Type* type) const;

    /**
     * Should be called by TypeCreator when all the fields are set up.
     * @note This can be called only once.
     */
    void FinishInitialization(const TypeInfo& info);

    static bool DefineInheritance(const Type* parent, const Type* child);

public: // TODO this must be private
    std::string mName;

    size_t mSize;
    size_t mAlignment;

    const Type* mParent;
    std::vector<const Type*> mChildTypes;
    Members mMembers;

private:
    bool mInitialized;
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
        // if you get this error, this means you didn't use NFE_DECLARE_TYPE(T)
        // or didn't include proper header containing these macros
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
