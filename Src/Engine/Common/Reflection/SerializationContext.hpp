#pragma once

#include "../nfCommon.hpp"
#include "Object.hpp"
#include "ReflectionMemberPath.hpp"
#include "ReflectionVariant.hpp"
#include "../Containers/HashMap.hpp"
#include "../Containers/DynArray.hpp"
#include "../Containers/StringView.hpp"
#include "../Containers/SharedPtr.hpp"
#include "../Memory/StaticBuffer.hpp"

namespace NFE {
namespace RTTI {

struct UnitTestHelper;

struct MemberTypeMismatchInfo
{
    MemberPath path;
    Variant readObject;
};

class NFCOMMON_API SerializationContext
{
    NFE_MAKE_NONCOPYABLE(SerializationContext)
    NFE_MAKE_NONMOVEABLE(SerializationContext)

public:
    static constexpr uint32 InvalidIndex = UINT32_MAX;

    enum class Stage
    {
        Mapping,
        Serialization,
        Deserialization,
    };

    static uint32 CurrentVersion;

    SerializationContext(uint32 version = CurrentVersion);
    ~SerializationContext();

    void SetUnitTestHelper(const UnitTestHelper* helper);

    NFE_FORCE_INLINE uint32 GetVersion() const { return mVersion; }
    NFE_FORCE_INLINE bool IsMapping() const { return mStage == Stage::Mapping; }
    NFE_FORCE_INLINE const UnitTestHelper* GetUnitTestHelper() const { return mUnitTestHelper; }

    NFE_FORCE_INLINE const Common::DynArray<Common::StringView>& GetMappedStrings() const { return mStringsTable; }
    NFE_FORCE_INLINE const Common::DynArray<ObjectPtr>& GetMappedObjects() const { return mObjectsTable; }

    void InitStage(Stage newStage);
    bool InitStringTable(Common::StaticBuffer&& stringBuffer);
    bool PushObject(const ObjectPtr& object);

    uint32 MapString(const Common::StringView str);
    uint32 MapObject(const ObjectPtr& object);
    bool IsObjectMapped(const IObject* object) const;
    uint32 GetObjectMappedIndex(const IObject* object) const;

    void OptimizeStringTable();
    void OptimizeObjectTable();

    bool UnmapString(const uint32 index, Common::StringView& outStr) const;
    bool UnmapObject(const uint32 index, ObjectPtr& outPtr) const;

    void PushMemberTypeMismatchInfo(const MemberPath& path, Variant&& readObject);

    const Common::DynArray<MemberTypeMismatchInfo>& GetMemberTypeMismatchInfos() const
    {
        return mMemberTypeMismatchInfos;
    }

private:

    struct MappingInfo
    {
        uint32 index = 0;
        uint32 occurenceCount = 0;
    };

    Stage mStage;
    uint32 mVersion;

    // strings collected during mapping phase
    Common::DynArray<Common::StringView> mStringsTable;
    Common::HashMap<Common::StringView, MappingInfo> mStringsMap;

    // objects collected during mapping phase
    Common::DynArray<ObjectPtr> mObjectsTable;
    Common::HashMap<const IObject*, MappingInfo> mObjectsMap;

    // used during deserialization
    Common::StaticBuffer mStringBuffer;

    // list of information about mismatched member types during deserialization
    Common::DynArray<MemberTypeMismatchInfo> mMemberTypeMismatchInfos;

    const UnitTestHelper* mUnitTestHelper;
};

} // namespace RTTI
} // namespace NFE
