/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of Scene class methods connected only with
 *         entities managing, events handling, etc.
 */

#include "PCH.hpp"
#include "SceneManager.hpp"
#include "Performance.hpp"
#include "Engine.hpp"
#include "SceneSegment.hpp"
#include "Systems/PhysicsSystem.hpp"
#include "Systems/RendererSystem.hpp"
#include "BVH.hpp"
#include "../nfCommon/Memory.hpp"
#include "../nfCommon/InputStream.hpp"
#include "../nfCommon/OutputStream.hpp"
#include "../nfCommon/Logger.hpp"

namespace NFE {
namespace Scene {

using namespace Math;
using namespace Renderer;
using namespace Resource;

SceneManager::SceneManager()
{
    mFocusSegment = nullptr;
    mDefaultCamera = nullptr;

    mPhysicsSystem.reset(new PhysicsSystem(this));
    mRendererSystem.reset(new RendererSystem(this));
}

SceneManager::~SceneManager()
{
    // TODO: temporary
    for (auto bodyTuple : mBodies)
    {
        std::get<1>(bodyTuple)->DisablePhysics();
    }
    mPhysicsSystem->Update(0.0f);

    //delete all entities
    for (auto pEntity : mEntities)
        delete pEntity;
    mEntities.clear();


    //delete links
    for (auto pLink : mLinks)
        delete pLink;
    mLinks.clear();

    //delete segments
    for (auto pSeg : mSegments)
        delete pSeg;
    mSegments.clear();
}

void SceneManager::SetEnvironment(const EnviromentDesc* pDesc)
{
    if (Common::MemoryCheck(pDesc) & ACCESS_READ)
        mEnvDesc = *pDesc;
}

void SceneManager::GetEnvironment(EnviromentDesc* pDesc) const
{
    if (Common::MemoryCheck(pDesc) & ACCESS_WRITE)
        *pDesc = mEnvDesc;
}

Entity* SceneManager::CreateEntity(Entity* pParent, const char* pName)
{
    if (pParent)
        if (mEntities.count(pParent) == 0)
        {
            LOG_ERROR("Entity (%p) does not belong to the scene (%p)", pParent, this);
            return 0;
        }

    Entity* pEntity;
    try
    {
        pEntity = new Entity();
        pEntity->mScene = this;
    }
    catch (std::exception& ba)
    {
        LOG_ERROR(ba.what());
        return 0;
    }

    mEntities.insert(pEntity);
    return pEntity;
}

Entity* SceneManager::GetEntityByName(const char* pName)
{
    if (mEntitiesMap.count(pName) == 0)
    {
        // no entities found
        return nullptr;
    }

    return mEntitiesMap[pName];
}

Result SceneManager::DeleteEntity(Entity* pEntity)
{
    if (mEntities.count(pEntity))
    {
        // remove from parent segment
        if (pEntity->mSegment != nullptr)
            pEntity->mSegment->mEntities.erase(pEntity);

        EventEntityDestroy event = {pEntity};
        mEventSystem.Push(SceneEvent::EntityDestroy, &event);

        mEntities.erase(pEntity);
        delete pEntity;
        return Result::OK;
    }

    LOG_ERROR("Entity (%p) does not belong to the scene (%p). Memory pointed by pEntity won't be released",
              pEntity, this);
    return Result::Error;
}

Result SceneManager::EnqueueDeleteEntity(Entity* pEntity, bool recursive)
{
    if (!pEntity)
        Result::CorruptedPointer;

    // remove from parent segment
    if (pEntity->mSegment != nullptr)
        pEntity->mSegment->mEntities.erase(pEntity);

    if (mEntities.count(pEntity))
    {
        mEntities.erase(pEntity);
        mEntitiesToRemove.push_back(pEntity);
        return Result::OK;
    }

    //LOG_ERROR("Entity (%p) does not belong to the scene (%p). Memory pointed by pEntity won't be released", pEntity, this);
    return Result::Error;
}



void SceneManager::SetDefaultCamera(CameraComponent* pCamera)
{
    // TODO: change default camera if the component is destroyed
    mDefaultCamera = pCamera;
}

CameraComponent* SceneManager::GetDefaultCamera() const
{
    return mDefaultCamera;
}

// Get segment pointer by world position
Segment* SceneManager::FindSegment(const Vector& position)
{
    for (auto pSeg : mSegments)
    {
        if ((pSeg->mDepth != X_SEGMENT_UNLOADED_DEPTH) && (pSeg->IsPointInside(position)))
            return pSeg;
    }

    return 0;
}

Segment* SceneManager::GetFocusSegment() const
{
    return mFocusSegment;
}

Result SceneManager::SetFocusSegment(Segment* pSegment)
{
    mFocusSegment = pSegment;

    // TODO: Arbitrary focus change (not only neighbours)

    for (auto pSeg : mSegments)
    {
        pSeg->mDepth = X_SEGMENT_UNLOADED_DEPTH;
    }

    pSegment->SetDepth(0, Vector());

    for (auto pSeg : mSegments)
    {
        pSeg->ApplyDepth();

        /*
        if (pSeg->mPrevDepth == X_SEGMENT_UNLOADED_DEPTH &&
            pSeg->mDepth < X_SEGMENT_UNLOADED_DEPTH) pSeg->Load();

        if (pSeg->mDepth == X_SEGMENT_UNLOADED_DEPTH &&
            pSeg->mPrevDepth < X_SEGMENT_UNLOADED_DEPTH) pSeg->Unload();

        pSeg->mPrevDepth = pSeg->mDepth;
        */
    }

    return Result::OK;
}

/*
    Create a new scene segment.
*/
Segment* SceneManager::CreateSegment(const char* pName, const Vector& size)
{
    Segment* pSegment = new Segment(this);
    if (!pSegment) return 0;

    // set parameters
    strcpy_s(pSegment->mName, pName);
    pSegment->mSize = size;

    mSegments.insert(pSegment);

    return pSegment;
}

/*
    Create a link between two segments.
    'offset' is displacement between pSegmentA and pSegmentB centers (order is important!).
*/
Result SceneManager::CreateLink(Segment* pFrom, Segment* pTo, const Vector& offset)
{
    if ((Common::MemoryCheck(pFrom) & ACCESS_READWRITE) != ACCESS_READWRITE)
    {
        LOG_ERROR("'pFrom' pointer (0x%08X) is corrupted", pFrom);
        return Result::CorruptedPointer;
    }

    if ((Common::MemoryCheck(pTo) & ACCESS_READWRITE) != ACCESS_READWRITE)
    {
        LOG_ERROR("'pTo' pointer (0x%08X) is corrupted", pTo);
        return Result::CorruptedPointer;
    }

    SegmentLink* pLink = new SegmentLink;
    if (!pLink) return Result::AllocationError;

    // assure (mFrom < mTo)
    if ((size_t)pFrom < (size_t)pTo)
    {
        pLink->from = pFrom;
        pLink->to = pTo;
        pLink->offset = offset;
    }
    else
    {
        pLink->from = pTo;
        pLink->to = pFrom;
        pLink->offset = -offset;
    }

    pFrom->mNeighbours.push_back(pLink);
    pTo->mNeighbours.push_back(pLink);

    mLinks.insert(pLink);
    return Result::OK;
}


/*
    Decode and create entity from input stream.
    Used, when a sector is loaded.
*/
Result SceneManager::DeserializeEntity(Common::InputStream* pStream, const Vector& offset,
                                       Entity** ppEntity)
{
    Entity* pEntity = new Entity();
    if (pEntity == nullptr) return Result::AllocationError;

    pEntity->mScene = this;

    // TODO
    // if (pEntity->Deserialize(pStream, offset) != Result::OK)
    // {
    //     delete pEntity; //free, the object won't be used anymore
    //     return Result::Error;
    // }

    mEntities.insert(pEntity);

    *ppEntity = pEntity;
    return Result::OK;
}

/*
    Check if default camera escaped focus segment.
    If so, switch fucus, unload/load segments and move coordinate system.
*/
void SceneManager::UpdateSegments()
{
    if (mDefaultCamera && mFocusSegment)
    {
        const TransformComponent& transform = mDefaultCamera->mOwner->mTransform;
        Vector cameraPos = transform.GetPosition();

        Box neighbourBox;
        Segment* pNeighbour = 0;
        Vector offset;
        bool switchFocus = false;

        if ((VectorGreaterMask(cameraPos, mFocusSegment->mSize) & 0x7) ||
                (VectorLessMask(cameraPos, -mFocusSegment->mSize) & 0x7) )
        {
            // iterate through neighbours
            for (auto pLink : mFocusSegment->mNeighbours)
            {
                if (pLink->from == mFocusSegment)
                {
                    pNeighbour = pLink->to;
                    offset = pLink->offset;
                }
                else
                {
                    pNeighbour = pLink->from;
                    offset = -pLink->offset;
                }

                neighbourBox.min = offset - pNeighbour->mSize;
                neighbourBox.max = offset + pNeighbour->mSize;

                // switch focus segment
                if (Intersect(neighbourBox, cameraPos))
                {
                    switchFocus = true;
                    break;
                }
            }
        }

        if (switchFocus)
        {
            for (auto pSegment : mSegments)
                pSegment->mOffset -= offset;

            // TODO : avoid SetPosition!!! Apply translation only!
            for (auto pEntity : mEntities)
            {
                (void)pEntity;
                // TODO
                //pEntity->SetPosition(pEntity->GetPosition() - offset);
            }

            SetFocusSegment(pNeighbour);
        }

    }
}

void SceneManager::UpdateSegmentForEntity(Entity* pEntity)
{
    Segment* pNewSegment = FindSegment(pEntity->mTransform.GetPosition());

    if ((pEntity->mSegment != pNewSegment) && (pNewSegment != nullptr)) // switch segment
    {
        if (pEntity->mSegment)
        {
            pEntity->mSegment->mEntities.erase(pEntity);
        }

        pNewSegment->mEntities.insert(pEntity);
        pEntity->mSegment = pNewSegment;
    }
}

void SceneManager::Update(float deltaTime)
{
    mEventSystem.Flush();

    UpdateSegments();
    mBodies.clear();


    // delete enqueued entities
    for (auto pEntity : mEntitiesToRemove)
    {
        EventEntityDestroy event = {pEntity};
        mEventSystem.Push(SceneEvent::EntityDestroy, &event);
        delete pEntity;
    }

    mEntitiesToRemove.clear();

    // segregate componets
    for (auto pEntity : mEntities)
    {
        for (auto pComp : pEntity->mComponents)
        {
            switch (pComp->GetType())
            {
                case ComponentType::Physics:
                    mBodies.push_back((BodyComponent*)pComp);
                    break;
            }
        }
    }

    if (mPhysicsSystem)
        mPhysicsSystem->Update(deltaTime);

    if (mRendererSystem)
        mRendererSystem->Update(deltaTime);
}

EventSystem* SceneManager::GetEventSystem()
{
    return &mEventSystem;
}

Result SceneManager::Serialize(Common::OutputStream* pStream, SerializationFormat format,
                               bool patch)
{
    char str[128];

    if (format == SerializationFormat::Xml)
    {
        using namespace rapidxml;
        xml_document<> doc;

        // xml declaration
        xml_node<>* decl = doc.allocate_node(node_declaration);
        decl->append_attribute(doc.allocate_attribute("version", "1.0"));
        decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
        doc.append_node(decl);

        // iterate segments
        for (auto pSegment : mSegments)
        {
            sprintf_s(str, "%g; %g; %g", pSegment->mSize.f[0], pSegment->mSize.f[1],
                      pSegment->mSize.f[2]); // use "g" formating to reduce text size

            // root node
            xml_node<>* root = doc.allocate_node(node_element, "segment");
            root->append_attribute(doc.allocate_attribute("name", pSegment->GetName()));
            root->append_attribute(doc.allocate_attribute("size", doc.allocate_string(str)));
            doc.append_node(root);
        }

        // iterate links
        for (auto pLink : mLinks)
        {
            sprintf_s(str, "%g; %g; %g", pLink->offset.f[0], pLink->offset.f[1],
                      pLink->offset.f[2]);

            // root node
            xml_node<>* root = doc.allocate_node(node_element, "link");
            root->append_attribute(doc.allocate_attribute("a", pLink->from->GetName()));
            root->append_attribute(doc.allocate_attribute("b", pLink->to->GetName()));
            root->append_attribute(doc.allocate_attribute("offset", doc.allocate_string(str)));
            doc.append_node(root);
        }

        // convert to string
        std::string xml_as_string;
        print(std::back_inserter(xml_as_string), doc);
        pStream->Write(xml_as_string.c_str(), xml_as_string.length());
    }
    else
    {
        LOG_ERROR("Unsupported serialization format");
        return Result::Error;
    }

    return Result::OK;
}

} // namespace Scene
} // namespace NFE
