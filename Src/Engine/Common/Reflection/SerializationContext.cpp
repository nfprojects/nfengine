#include "PCH.hpp"
#include "SerializationContext.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;


uint32 SerializationContext::CurrentVersion = 0;

static const SharedPtr<IObject> c_dummyObject;

SerializationContext::SerializationContext(uint32 version)
    : mStage(Stage::Mapping)
    , mVersion(version)
    , mUnitTestHelper(nullptr)
{ }

SerializationContext::~SerializationContext() = default;

void SerializationContext::SetUnitTestHelper(const UnitTestHelper* helper)
{
    mUnitTestHelper = helper;
}

void SerializationContext::InitStage(Stage newStage)
{
    mStage = newStage;
}

uint32 SerializationContext::MapString(const StringView str)
{
    NFE_ASSERT(mStage == Stage::Mapping || mStage == Stage::Serialization, "Invalid stage");

    const auto iter = mStringsMap.Find(str);
    if (iter != mStringsMap.End())
    {
        if (mStage == Stage::Mapping)
        {
            iter->second.occurenceCount++;
        }

        const uint32 index = iter->second.index;
        NFE_ASSERT(str == mStringsTable[index], "String table is corrupted");
        return index;
    }

    NFE_ASSERT(mStage == Stage::Mapping, "SerializationContext: String '%.*s' not found during serialization stage", str.Length(), str.Data());

    MappingInfo newMappingInfo;
    newMappingInfo.index = mStringsTable.Size();
    newMappingInfo.occurenceCount = 1;

    mStringsTable.PushBack(str);
    mStringsMap.Insert(str, newMappingInfo);
    NFE_ASSERT(mStringsTable.Size() == mStringsMap.Size(), "Invalid string table size");

    return newMappingInfo.index;
}

uint32 SerializationContext::MapObject(const ObjectPtr& object)
{
    NFE_ASSERT(object, "Tring to map nullptr");
    NFE_ASSERT(mStage == Stage::Mapping || mStage == Stage::Serialization, "Invalid stage");

    const auto iter = mObjectsMap.Find(object.Get());
    if (iter != mObjectsMap.End())
    {
        if (mStage == Stage::Mapping)
        {
            iter->second.occurenceCount++;
        }

        const uint32 index = iter->second.index;
        NFE_ASSERT(object == mObjectsTable[index], "Object table is corrupted");
        return index;
    }

    NFE_ASSERT(mStage == Stage::Mapping, "SerializationContext: Object not found during serialization stage");

    MappingInfo newMappingInfo;
    newMappingInfo.index = mObjectsTable.Size();
    newMappingInfo.occurenceCount = 1;

    mObjectsTable.PushBack(object);
    mObjectsMap.Insert(object.Get(), newMappingInfo);
    NFE_ASSERT(mObjectsTable.Size() == mObjectsMap.Size(), "Invalid object map size");

    return newMappingInfo.index;
}

bool SerializationContext::IsObjectMapped(const IObject* object) const
{
    NFE_ASSERT(object, "Tring to map nullptr");
    NFE_ASSERT(mStage == Stage::Mapping || mStage == Stage::Serialization, "Invalid stage");

    return mObjectsMap.Exists(object);
}


void SerializationContext::OptimizeStringTable()
{
    // TODO
}

void SerializationContext::OptimizeObjectTable()
{
    // TODO
}

bool SerializationContext::UnmapString(const uint32 index, Common::StringView& outStr) const
{
    if (index < mStringsTable.Size())
    {
        outStr = mStringsTable[index];
        return true;
    }
    return false;
}

bool SerializationContext::UnmapObject(const uint32 index, ObjectPtr& outPtr) const
{
    if (index < mObjectsTable.Size())
    {
        outPtr = mObjectsTable[index];
        return true;
    }
    return false;
}

bool SerializationContext::InitStringTable(Buffer&& newStringBuffer)
{
    mStringsTable.Clear();
    mStringBuffer = std::move(newStringBuffer);

    const char* stringBuffer = reinterpret_cast<const char*>(mStringBuffer.Data());
    const size_t stringBufferSize = mStringBuffer.Size();

    size_t prevOffset = 0;
    uint32 numStrings = 0;

    for (size_t i = 0; i < stringBufferSize; ++i)
    {
        if (stringBuffer[i] == 0)
        {
            const size_t stringLen = i - prevOffset;
            if (stringLen > StringView::MAX_LENGTH())
            {
                mStringsTable.Clear();
                return false;
            }

            const StringView strView(stringBuffer + prevOffset, (uint32)stringLen);
            mStringsTable.PushBack(strView);

            numStrings++;
            prevOffset = i + 1;
        }
    }

    if (prevOffset < stringBufferSize)
    {
        const size_t stringLen = stringBufferSize - prevOffset;
        if (stringLen > StringView::MAX_LENGTH())
        {
            mStringsTable.Clear();
            return false;
        }

        const StringView strView(stringBuffer + prevOffset, (uint32)stringLen);
        mStringsTable.PushBack(strView);

        numStrings++;
    }

    if (mStringsTable.Size() != numStrings)
    {
        return false;
    }

    return true;
}

bool SerializationContext::PushObject(const ObjectPtr& object)
{
    mObjectsTable.PushBack(object);

    return true;
}

void SerializationContext::PushMemberTypeMismatchInfo(const MemberPath& path, Variant&& readObject)
{
    mMemberTypeMismatchInfos.PushBack({ path, std::move(readObject) });
}

} // namespace RTTI
} // namespace NFE
