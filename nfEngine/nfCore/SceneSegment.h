/**
    NFEngine project

    \file   SceneSegment.h
    \brief  Scene segment declaration.
*/

#pragma once
#include "Core.h"
#include "Aligned.h"
#include "Entity.h"
#include "..\nfCommon\OutputStream.h"

namespace NFE {
namespace Scene {

// maximum name of scene sector
#define X_SEGMENT_MAX_NAME_LENGTH 128
//#define X_SEGMENT_FLAG_LOADED (1<<16)
#define X_SEGMENT_UNLOADED_DEPTH 0xFFFFFFFF

//connection between two sectors
NFE_ALIGN(16)
struct SegmentLink : public Util::Aligned
{
    Segment* from;
    Segment* to;
    Math::Vector offset;
};

/**
 * Structure used during serialization.
 */
#pragma pack(push, 1)
struct SegmentDesc
{
    Math::Float3 size;
    //
};
#pragma pack(pop)


/**
 * Class describing a scene segment.
 * @details A scene segment is a list of entities inside, dimensions and neighbours. A state
            of a scene segment can be loaded and saveed to file or memory.
*/
NFE_ALIGN(16)
class CORE_API Segment : public Util::Aligned
{
    friend class SceneManager;

    // Sector box dimensions. TODO in future: more complex shapes
    Math::Vector mSize;

    // Sector center offset relative to scene center (0, 0, 0)
    Math::Vector mOffset;

    // owner
    SceneManager* mScene;

    /*
        depth = 0 means that it's focus segment,
        depth = 1 - focus segment neighbour, etc.
        depth = 0xFFFFFFFF - segment unloaded
    */
    uint32 mDepth;
    uint32 mPrevDepth;


    // neighbouring segments
    std::vector<SegmentLink*> mNeighbours;

    //list of entities inside the sector existing in the scene 'mScene'
    std::set<Entity*> mEntities;

    /*
        Raw binary buffer (stream) containing inforamtion about entities inside the segment.
        When segment is being loaded, entities are deserialized from that buffer.
        When segment is being unloaded, entities are serialized to this buffer.
    */
    Common::BufferOutputStream mDataStream;

    // Segment name. Avoid dynamic allocations
    char mName[X_SEGMENT_MAX_NAME_LENGTH];


    void ApplyDepth();
    void Load();
    void Unload();

    void SetDepth(uint32 newDepth, const Math::Vector& offset);


public:
    Segment(SceneManager* pOwner);
    virtual ~Segment();

    const char* GetName() const;

    Result AddEntityFromRawBuffer(const void* pData, size_t dataSize);

    /**
     * Serialize segment data (including entities from mDataStream)
     */
    Result Serialize(Common::OutputStream* pStream) const;

    /**
     * Check if "worldPos" belongs to this segment
     */
    bool IsPointInside(const Math::Vector& worldPos) const;
};

} // namespace Scene
} // namespace NFE
