/**
 * @file   SceneSegment.cpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Scene segment definition.
 */

#include "PCH.hpp"
#include "SceneSegment.hpp"
#include "Entity.hpp"
#include "SceneManager.hpp"
#include "MeshComponent.hpp"
#include "Light.hpp"
#include "Body.hpp"
#include "Camera.hpp"
#include "../nfCommon/InputStream.hpp"
#include "../nfCommon/OutputStream.hpp"
#include "../nfCommon/Logger.hpp"

namespace NFE {
namespace Scene {

using namespace Math;

Segment::Segment(SceneManager* pOwner)
{
    mSize = Vector();
    mOffset = Vector();

    mScene = pOwner;
    mPrevDepth = mDepth = X_SEGMENT_UNLOADED_DEPTH;
    strcpy_s(mName, "untitled_segment");
}

Segment::~Segment()
{

}

const char* Segment::GetName() const
{
    return mName;
}

bool Segment::IsPointInside(const Vector& worldPos) const
{
    // position in segment space
    Vector segPos = worldPos - mOffset;

    return (((VectorLessMask(segPos, mSize) & 0x7) == 0x7) &&
            ((VectorGreaterMask(segPos, -mSize) & 0x7) == 0x7));
}

void Segment::SetDepth(uint32 newDepth, const Vector& offset)
{
    if (newDepth > 1)
        return;

    if (newDepth >= mDepth)
        return;

    mDepth = newDepth;
    mOffset = offset;

    // TODO: update depth of neighbours and load/unload if needed
    for (auto pNeighbour : mNeighbours)
    {
        if (pNeighbour->from == this)
            pNeighbour->to->SetDepth(newDepth + 1, offset + pNeighbour->offset);
        else
            pNeighbour->from->SetDepth(newDepth + 1, offset - pNeighbour->offset);
    }
}

void Segment::ApplyDepth()
{
    if (mPrevDepth == X_SEGMENT_UNLOADED_DEPTH && mDepth < X_SEGMENT_UNLOADED_DEPTH)
        Load();
    else if (mDepth == X_SEGMENT_UNLOADED_DEPTH && mPrevDepth < X_SEGMENT_UNLOADED_DEPTH)
        Unload();

    mPrevDepth = mDepth;
}

void Segment::Load()
{
    LOG_INFO("Loading segment '%s'...", mName);
    mEntities.clear();

    Common::BufferInputStream inputStream(mDataStream.GetData(), mDataStream.GetSize());

    Entity* pEntity;
    while (mScene->DeserializeEntity(&inputStream, mOffset, &pEntity) == Result::OK)
    {
        mEntities.insert(pEntity);
        pEntity->mSegment = this;
    }

    mDataStream.Clear();
}

void Segment::Unload()
{
    LOG_INFO("Unloading segment '%s'...", mName);
    mDataStream.Clear();

    // create local copy of mEntities (SceneManager::EnqueueDeleteEntity can modify it)
    std::set<Entity*> entitiesCopy = mEntities;

    // serialze and destroy all entities inside this segment
    for (const auto pEntity : entitiesCopy)
    {
        if ((pEntity->GetParent() == nullptr) && (mScene->mEntities.count(pEntity)))
        {
            pEntity->Serialize(&mDataStream, mOffset);
            mScene->EnqueueDeleteEntity(pEntity, true);
        }
    }

    mEntities.clear();
}

Result Segment::AddEntityFromRawBuffer(const void* pData, size_t dataSize)
{
    if (mDataStream.Write(pData, dataSize) == dataSize)
        return Result::OK;

    return Result::Error;
}

// serialize segment data (including entities from mDataStream)
Result Segment::Serialize(Common::OutputStream* pStream) const
{
    if (pStream == nullptr)
        return Result::CorruptedPointer;

    SegmentDesc desc;
    VectorStore(mSize, &desc.size);

    if (pStream->Write(&desc, sizeof(desc)) != sizeof(desc))
        return Result::Error;


    const void* pData = mDataStream.GetData();
    size_t size = mDataStream.GetSize();
    if (pStream->Write(pData, size) != size)
        return Result::Error;

    return Result::OK;
}

} // namespace Scene
} // namespace NFE
