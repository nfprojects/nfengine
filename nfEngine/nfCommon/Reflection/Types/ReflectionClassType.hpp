/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's Type class.
 */

#pragma once

#include "../../nfCommon.hpp"
#include "ReflectionType.hpp"
#include "../ReflectionMember.hpp"

#include <vector>


namespace NFE {
namespace RTTI {

class ClassType;

struct ClassTypeInfo : public TypeInfo
{
    const ClassType* parent;
    std::vector<const ClassType*> childTypes;
    std::vector<Member> members;
    bool isTriviallyConstructible;

    ClassTypeInfo()
        : TypeInfo()
        , parent(nullptr)
        , isTriviallyConstructible(true)
    { }
};

/**
 * Simple, polymorphic or abstract class type.
 */
class NFCOMMON_API ClassType : public Type
{
    NFE_MAKE_NONCOPYABLE(ClassType);

public:
    template <typename T>
    friend class TypeCreator;

    using Members = std::vector<Member>;
    using Children = std::vector<const ClassType*>;

    ClassType(const ClassTypeInfo& info);

    /**
     * Get parent class.
     * @note    Will return nullptr for root classes.
     */
    const ClassType* GetParent() const { return mParent; }

    /**
     * Enumerate all subtypes of this type (including self).
     */
    void ListSubtypes(std::vector<const ClassType*>& outTypes) const;

    /**
     * Get number of member (including members that come from parent type).
     */
    size_t GetNumOfMembers() const;

    /**
     * Enumerate all members (including members that come from parent type).
     */
    void ListMembers(std::vector<Member>& outMembers) const;

    // Type interface implementation
    bool IsA(const Type* baseType) const override;
    bool Serialize(const void* object, Common::Config& config, Common::ConfigValue& outValue) const override;
    bool Deserialize(void* outObject, const Common::Config& config, const Common::ConfigValue& value) const override;

private:
    // list of members (NOT INCLUDING derived members)
    Members mMembers;

    // parent class type
    const ClassType* mParent;

    // list of class types that derive directly from this type
    Children mChildTypes;
};


} // namespace RTTI
} // namespace NFE
