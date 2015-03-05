/**
 * @file    Buffer.hpp
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Types.hpp"

namespace NFE {
namespace Renderer {

/**
 * Description of renderer's buffer
 */
struct BufferDesc
{
    BufferType type;
    BufferAccess access;
    const void* initialData;
    size_t size;
};

/**
 * GPU data buffer which can act as a vertex, index or a constant buffer.
 */
class IBuffer
{
public:
    virtual ~IBuffer() {}

    /**
     * Write data from CPU memory to the buffer.
     * @param offset Offset in the GPU buffer in bytes.
     * @param size   Bytes to write.
     * @param data   Pointer to data to write.
     */
    virtual void Write(size_t offset, size_t size, const void* data) = 0;

    /**
    * Read data from the GPU buffer to CPU memory.
    * @param offset Offset in the GPU buffer in bytes.
    * @param size   Bytes to read.
    * @param data   Where data should be written.
    */
    virtual void Read(size_t offset, size_t size, void* data) = 0;
};

} // namespace Renderer
} // namespace NFE
