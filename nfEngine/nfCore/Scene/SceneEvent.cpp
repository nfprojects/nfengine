/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of scene events system.
 */

#include "PCH.hpp"
#include "SceneEvent.hpp"
#include "nfCommon/Logger.hpp"

namespace NFE {
namespace Scene {

// 1KB of initial buffer
const size_t g_EventBuffInitialCapacity = 1024;
const size_t g_EventsNumber = 32; // this should be calculated at compile time

// define static members here
size_t g_EventsSizes[g_EventsNumber] = {0};
const char* g_EventsNames[g_EventsNumber]  = {nullptr};

#define X_REGISTER_EVENT(id, s) \
    int g_RegisterEvent_##s##_Func() \
    { \
        g_EventsSizes[(int)id] = sizeof(s); \
        g_EventsNames[(int)id] = #s; \
        return (int)id; \
    } \
    static int g_RegisterEvent_##s##_Var = g_RegisterEvent_##s##_Func();


/// Register all the scene events
X_REGISTER_EVENT(SceneEvent::EntityDestroy, EventEntityDestroy)
X_REGISTER_EVENT(SceneEvent::BodyEscape, EventBodyEscape)
X_REGISTER_EVENT(SceneEvent::BodyCollide, EventBodyCollide)



EventSystem::EventSystem()
{
    mCapacity = g_EventBuffInitialCapacity;
    mSize = 0;
    mSeekPos = 0;

    mBuffer = malloc(mCapacity);
    if (mBuffer == NULL)
    {
        LOG_ERROR("Events buffer allocation failed");
    }
}

EventSystem::~EventSystem()
{
    if (mBuffer)
    {
        free(mBuffer);
    }
}

bool EventSystem::Push(SceneEvent eventID, const void* pData)
{
    size_t dataSize = g_EventsSizes[(int)eventID];

    size_t newCapacity = mCapacity;
    while (mSize + dataSize + 1 > newCapacity)
        newCapacity *= 2;

    if ((newCapacity != mCapacity) || (mBuffer == NULL))
    {
        void* pNewBuffer = realloc(mBuffer, newCapacity);

        if (pNewBuffer == NULL)
        {
            LOG_ERROR("Failed to extend events buffer");
            return false;
        }

        mBuffer = pNewBuffer;
        mCapacity = newCapacity;
    }

    unsigned char id = (unsigned char)eventID;
    memcpy((char*)mBuffer + mSize, &id, 1);
    memcpy((char*)mBuffer + mSize + 1, pData, dataSize);
    mSize += dataSize + 1;
    return true;
}

bool EventSystem::Pop(SceneEvent* pEventID, void** ppData)
{
    if (mSeekPos >= mSize)
        return false;

    unsigned char id = ((unsigned char*)mBuffer)[mSeekPos];
    size_t dataSize = g_EventsSizes[id];
    *pEventID = (SceneEvent)id;
    *ppData = ((unsigned char*)mBuffer) + mSeekPos + 1;
    mSeekPos += dataSize + 1;

    return true;
}

void EventSystem::Flush()
{
    void* newBuffer = realloc(mBuffer, g_EventBuffInitialCapacity);
    if (newBuffer == nullptr)
    {
        free(mBuffer);
        mBuffer = nullptr;
        LOG_ERROR("Events buffer allocation failed");
        return;
    }

    mBuffer = newBuffer;
    mCapacity = g_EventBuffInitialCapacity;
    mSize = 0;
    mSeekPos = 0;
}

} // namespace Scene
} // namespace NFE
