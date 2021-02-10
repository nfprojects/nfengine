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
    size_t size;
    uint32 structSize;
    const char* debugName;   //< optional debug name

    BufferDesc()
        : mode(ResourceAccessMode::GPUOnly)
        , usage(0u)
        , size(0u)
        , structSize(0u)
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
 * GPU data buffer.
 */
class IBuffer
{
public:
    virtual ~IBuffer() {}

    /**
     * Map the buffer into CPU memory virtual space.
     */
    virtual void* Map(size_t size = 0u, size_t offset = 0u) = 0;

    /**
     * Unmap the buffer from CPU memory virtual space.
     */
    virtual void Unmap() = 0;
};

} // namespace Renderer
} // namespace NFE
