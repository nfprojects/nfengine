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
    friend class ResourceBindingInstance;

    size_t mSize;
    BufferType mType;
    BufferMode mMode;
    D3DPtr<ID3D11Buffer> mBuffer;

public:
    Buffer();
    bool Init(const BufferDesc& desc);
};

} // namespace Renderer
} // namespace NFE
