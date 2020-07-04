#include "../PCH.hpp"
#include "ResourceState.hpp"


namespace NFE {
namespace Renderer {

void ResourceState::RegisterResource(Texture* resource)
{
    NFE_UNUSED(resource);
}

void ResourceState::SubmitChanges(CommandBatch& commands)
{
    NFE_UNUSED(commands);
}

} // namespace Renderer
} // namespace NFE
