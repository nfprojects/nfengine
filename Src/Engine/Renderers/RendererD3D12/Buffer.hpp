/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 renderer's buffer.
 */

#pragma once

#include "../RendererCommon/Buffer.hpp"

#include "Resource.hpp"
#include "Common.hpp"


namespace NFE {
namespace Renderer {

class Buffer : public IBuffer, public Resource
{
    uint32 mSize;

    // Upload data (used for Static buffers only)
    bool UploadData(const BufferDesc& desc);

public:
    Buffer();
    ~Buffer();

    bool Init(const BufferDesc& desc);

    NFE_INLINE ID3D12Resource* GetResource() const
    {
        return mResource.Get();
    }

    NFE_INLINE uint32 GetSize() const
    {
        return mSize;
    }

    // get allocated memory size
    NFE_INLINE uint32 GetRealSize() const
    {
        return (mSize + 255) & ~255;
    }
};

} // namespace Renderer
} // namespace NFE
