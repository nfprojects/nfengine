/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 renderer's buffer.
 */

#pragma once

#include "../RendererInterface/Buffer.hpp"
#include "Common.hpp"

namespace NFE {
namespace Renderer {

class Buffer : public IBuffer
{
    friend class CommandBuffer;
    friend class ResourceBindingInstance;

    size_t mSize;
    void* mData; // mapped data
    BufferType mType;
    BufferAccess mAccess;

    D3DPtr<ID3D12Resource> mResource;

public:
    Buffer();
    ~Buffer();
    bool Init(const BufferDesc& desc);
};

} // namespace Renderer
} // namespace NFE
