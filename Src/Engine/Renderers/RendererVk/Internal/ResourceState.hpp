#pragma once

#include "API/Texture.hpp"
#include "CommandBatch.hpp"


namespace NFE {
namespace Renderer {


class ResourceState
{
    struct Entry
    {
        Texture* resource;
        VkImageLayout layout;

        Entry* next;
        Entry* prev;
    };



public:
    ResourceState() = default;
    ~ResourceState() = default;

    void RegisterResource(Texture* resource);
    void SubmitChanges(CommandBatch& commands);
};


} // namespace Renderer
} // namespace NFE
