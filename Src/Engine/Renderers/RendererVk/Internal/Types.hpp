#pragma once

#include <Engine/Common/nfCommon.hpp>


namespace NFE {
namespace Renderer {
namespace Internal {


using ResourceID = uint32;

enum class ResourceType: uint8
{
    Buffer = 0,
    Texture,
    Backbuffer,
    Max
};


} // namespace Internal
} // namespace Renderer
} // namespace NFE
