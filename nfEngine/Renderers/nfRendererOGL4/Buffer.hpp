/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of OpenGL 4 renderer's buffer.
 */

#pragma once

#include "../RendererInterface/Buffer.hpp"
#include "Defines.hpp"

namespace NFE {
namespace Renderer {

class Buffer : public IBuffer
{
    friend class CommandBuffer;

    size_t mSize;
    BufferType mType;
    BufferAccess mAccess;
    GLuint mBuffer;

public:
    Buffer();
    ~Buffer();
    bool Init(const BufferDesc& desc);
    void Write(size_t offset, size_t size, const void* data);
    void Read(size_t offset, size_t size, void* data);
};

} // namespace Renderer
} // namespace NFE
