/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of scene events system.
 */

#pragma once

#include "../Core.hpp"
#include "EntityManager.hpp"
#include "nfCommon/Aligned.hpp"

namespace NFE {
namespace Scene {

enum class SceneEvent
{
    /// Entity events:
    EntityDestroy = 0,

    /// Body events:
    BodyEscape,
    BodyCollide,
};

struct EventEntityDestroy
{
    EntityID entity;
};

struct EventBodyEscape
{
    BodyComponent* body;
};

struct EventBodyCollide
{
    BodyComponent* bodyA;
    BodyComponent* bodyB;
    // TODO: more attributes (velocity, position, normal, etc.)
};

class CORE_API EventSystem
{
    // dynamically expanding buffer holding all the events
    void* mBuffer;

    // number of bytes used by rhe buffer
    size_t mSize;

    // total buffer capacity (in bytes)
    size_t mCapacity;

    // pointer used by Pop() method
    size_t mSeekPos;

public:
    EventSystem();
    ~EventSystem();

    // Insert a new event. Should be used only by the engine
    bool Push(SceneEvent eventID, const void* pData);

    /**
     * Get an event from the buffer.
     * @param[out] pEventID
     * @param[out] ppData Place to write event data. Data size depends on event type.
     * @return false if the buffer is empty.
    */
    bool Pop(SceneEvent* pEventID, void** ppData);

    /**
     * Clear events buffer.
     */
    void Flush();
};

} // namespace Scene
} // namespace NFE
