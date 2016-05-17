/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 11 renderer's buffer.
 */

#pragma once

#include "../RendererInterface/Buffer.hpp"
#include "Common.hpp"

namespace NFE {
namespace Renderer {

class Buffer : public IBuffer
{
    friend class CommandBuffer;
    friend class Device;

    size_t mSize;
    BufferType mType;
    BufferAccess mAccess;
    D3DPtr<ID3D11Buffer> mBuffer;

public:
    Buffer();
    bool Init(const BufferDesc& desc);
    void Write(size_t offset, size_t size, const void* data);
    void Read(size_t offset, size_t size, void* data);
};

} // namespace Renderer
} // namespace NFE
