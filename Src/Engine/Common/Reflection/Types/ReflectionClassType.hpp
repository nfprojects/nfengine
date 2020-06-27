/**
 * @file
 * @author Witek902
 * @brief  Declaration of reflection system's ClassType class.
 */

#pragma once

#include "../../nfCommon.hpp"
#include "../../Containers/DynArray.hpp"
#include "ReflectionType.hpp"
#include "../ReflectionTypeResolver.hpp"
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
    static const Common::StringView TYPE_MARKER;

    using Members = Common::DynArray<Member>;
    using Children = Common::DynArray<const ClassType*>;

    ClassType();

    /**
     * Get parent class.
     * @note    Will return nullptr for root classes.
     */
    const ClassType* GetParent() const { return mParent; }

    /**
     * Enumerate all subtypes of this type (including self).
     */
    void ListSubtypes(Children& outTypes, bool skipAbstractTypes = false) const;
    void ListSubtypes(const std::function<void(const ClassType*)>& func, bool skipAbstractTypes = false) const;

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

    // Compare members in two objects and return list of differing ones
    void CollectDifferingMemberList(const void* objectA, const void* objectB, Common::DynArray<const Member*>& outMemberList) const;

    // Type interface implementation
    virtual void PrintInfo() const override;
    virtual bool IsA(const Type* baseType) const override;
    virtual bool Serialize(const void* object, Common::IConfig& config, Common::ConfigValue& outValue, SerializationContext& context) const override;
    virtual bool SerializeBinary(const void* object, Common::OutputStream* stream, SerializationContext& context) const override;
    virtual bool Deserialize(void* outObject, const Common::IConfig& config, const Common::ConfigValue& value, SerializationContext& context) const override;
    virtual bool DeserializeBinary(void* outObject, Common::InputStream& stream, SerializationContext& context) const override;
    virtual bool Compare(const void* objectA, const void* objectB) const override;
    virtual bool Clone(void* destObject, const void* sourceObject) const override;

private:

    virtual void OnInitialize(const TypeInfo& info) override;

    // parent class type
    const ClassType* mParent;

    // list of class types that derive directly from this type
    Children mChildTypes;

    // list of members (NOT INCLUDING derived members)
    Members mMembers;

    // serialize directly to an existing ConfigObject structure
    bool SerializeDirectly(const void* object, Common::IConfig& config, Common::ConfigObject& outObject, SerializationContext& context) const;

    // deserialize member of this class (or derived one) by name
    bool DeserializeMember(void* outObject, const Common::StringView memberName, const Common::IConfig& config, const Common::ConfigValue& value, SerializationContext& context) const;
};


/**
 * Helper class used for ClassType initialization.
 */
struct ClassTypeInfo : public TypeInfo
{
    const ClassType* parent = nullptr;
    ClassType::Children childTypes;
    Common::DynArray<Member> members;
};

/**
 * Template class used to generate Type class instances.
 * This template will be specialized in NFE_DECLARE_TYPE macro.
 */
template <typename T>
class TypeCreator
{
public:
    using TypeClass = ClassType;
    using TypeInfoClass = ClassTypeInfo;

    static Type* CreateType()
    {
        return new ClassType();
    }

    static void InitializeType(Type* type)
    {
        ClassTypeInfo typeInfo;
        typeInfo.kind = TypeKind::SimpleClass; // can be overridden in FinishInitialization
        typeInfo.size = sizeof(T);
        typeInfo.alignment = std::alignment_of_v<T>;
        typeInfo.constructor = GetObjectConstructor<T>();
        typeInfo.destructor = GetObjectDestructor<T>();

        TypeCreator creator;
        creator.FinishInitialization(typeInfo);
        static_cast<ClassType*>(type)->Initialize(typeInfo);
    }

    void FinishInitialization(TypeInfoClass& typeInfo);
};


} // namespace RTTI
} // namespace NFE
