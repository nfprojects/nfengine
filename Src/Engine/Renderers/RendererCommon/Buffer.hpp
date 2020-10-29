/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Types.hpp"

#define NFE_RENDERER_BUFFER_USAGE_INDEX_BUFFER              (1<<0)
#define NFE_RENDERER_BUFFER_USAGE_VERTEX_BUFFER             (1<<1)
#define NFE_RENDERER_BUFFER_USAGE_CONSTANT_BUFFER           (1<<2)
#define NFE_RENDERER_BUFFER_USAGE_STRUCT_BUFFER             (1<<3)
#define NFE_RENDERER_BUFFER_USAGE_WRITABLE_STRUCT_BUFFER    (1<<4)

namespace NFE {
namespace Renderer {

/**
 * Description of renderer's buffer
 */
struct BufferDesc
{
    ResourceAccessMode mode;
    uint32 usage;
    const void* initialData;
    size_t size;
    const char* debugName;   //< optional debug name

    BufferDesc()
        : mode(ResourceAccessMode::Static)
        , usage(0u)
        , initialData(nullptr)
        , size(0u)
        , debugName(nullptr)
    {}
};

/**
 * Buffer's view for binding
 */
struct BufferView
{
    uint32 firstElement = 0;
    uint32 numElements = UINT32_MAX;
};

/**
 * GPU data buffer which can act as a vertex, index or a constant buffer.
 */
class IBuffer
{
public:
    virtual ~IBuffer() {}
};

} // namespace Renderer
} // namespace NFE
