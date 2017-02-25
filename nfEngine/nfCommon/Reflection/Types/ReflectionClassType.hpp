/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's ClassType class.
 */

#pragma once

#include "../../nfCommon.hpp"
#include "../../Containers/StringView.hpp"
#include "../../Containers/DynArray.hpp"
#include "ReflectionType.hpp"
#include "../ReflectionMember.hpp"


namespace NFE {
namespace RTTI {

struct ClassTypeInfo;

/**
 * Simple, polymorphic or abstract class type.
 */
class NFCOMMON_API ClassType : public Type
{
    NFE_MAKE_NONCOPYABLE(ClassType)

public:
    template <typename T>
    friend class TypeCreator;

    // type marker used in config, when serializing polymorphic types
    static const char* TYPE_MARKER;

    using Members = Common::DynArray<Member>;
    using Children = Common::DynArray<const ClassType*>;

    ClassType(const ClassTypeInfo& info);

    /**
     * Get parent class.
     * @note    Will return nullptr for root classes.
     */
    const ClassType* GetParent() const { return mParent; }

    /**
     * Enumerate all subtypes of this type (including self).
     */
    void ListSubtypes(Children& outTypes) const;

    /**
     * Get number of member (including members that come from parent type).
     */
    size_t GetNumOfMembers() const;

    /**
     * Enumerate all members (including members that come from parent type).
     */
    void ListMembers(Members& outMembers) const;

    /**
     * Find member by name.
     * @return  Member pointer of nullptr if not found.
     */
    const Member* FindMember(const Common::StringView name) const;

    // Type interface implementation
    void PrintInfo() const override;
    bool IsA(const Type* baseType) const override;
    bool Serialize(const void* object, Common::Config& config, Common::ConfigValue& outValue) const override;
    bool Deserialize(void* outObject, const Common::Config& config, const Common::ConfigValue& value) const override;

private:
    // parent class type
    const ClassType* mParent;

    // list of class types that derive directly from this type
    Children mChildTypes;

    // list of members (NOT INCLUDING derived members)
    Members mMembers;

    // serialize directly to an existing ConfigObject structure
    bool SerializeDirectly(const void* object, Common::Config& config, Common::ConfigObject& outObject) const;

    // deserialize member of this class (or derived one) by name
    bool DeserializeMember(void* outObject, const char* memberName, const Common::Config& config, const Common::ConfigValue& value) const;
};


/**
 * Helper class used for ClassType initialization.
 */
struct ClassTypeInfo : public TypeInfo
{
    const ClassType* parent;
    ClassType::Children childTypes;
    Common::DynArray<Member> members;

    ClassTypeInfo()
        : TypeInfo()
        , parent(nullptr)
    { }
};


} // namespace RTTI
} // namespace NFE
