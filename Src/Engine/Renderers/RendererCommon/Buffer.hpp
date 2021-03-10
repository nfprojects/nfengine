/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Types.hpp"

namespace NFE {
namespace Renderer {

/**
 * Buffer's usage flags. The buffer usage must be determined upon creation.
 */
enum class BufferUsageFlag : uint8
{
    IndexBuffer     = NFE_FLAG(0),
    VertexBuffer    = NFE_FLAG(1),
    ConstantBuffer  = NFE_FLAG(2),
    ReadonlyStruct  = NFE_FLAG(3),
    WritableStruct  = NFE_FLAG(4),
};

DEFINE_ENUM_OPERATORS(BufferUsageFlag);

/**
 * Description of renderer's buffer
 */
struct BufferDesc
{
    ResourceAccessMode mode;
    BufferUsageFlag usage;
    size_t size;
    uint32 structSize;
    const char* debugName;   //< optional debug name

    BufferDesc()
        : mode(ResourceAccessMode::Immutable)
        , usage(static_cast<BufferUsageFlag>(0))
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
