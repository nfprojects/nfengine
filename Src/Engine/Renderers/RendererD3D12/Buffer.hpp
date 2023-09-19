/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 renderer's buffer.
 */

#pragma once

#include "../RendererCommon/Buffer.hpp"

#include "Descriptors.hpp"
#include "Resource.hpp"
#include "Common.hpp"

namespace NFE {
namespace Renderer {

class Buffer : public IBuffer, public Resource
{
public:
    Buffer();
    ~Buffer();

    virtual void* Map(size_t size, size_t offset) override;
    virtual void Unmap() override;

    bool Init(const BufferDesc& desc);

    // get or create descriptor of a buffer view
    [[nodiscard]] DescriptorID GetDescriptor(DescriptorType type, const BufferView& view);

    NFE_FORCE_INLINE ID3D12Resource* GetResource() const
    {
        return mResource.Get();
    }

    NFE_FORCE_INLINE uint32 GetSize() const
    {
        return mSize;
    }

    NFE_FORCE_INLINE uint32 GetStructSize() const
    {
        return mStructureSize;
    }

    // get allocated memory size
    NFE_FORCE_INLINE uint32 GetRealSize() const
    {
        return (mSize + 255) & ~255;
    }

private:

    uint32 mSize;
    uint32 mStructureSize;

    // TODO this wastes a lot of space...
    // maybe introduce "default" descriptor created when buffer is created?
    struct DescriptorInfo
    {
        DescriptorType type;
        BufferView view;
        DescriptorID descriptor = UINT32_MAX;
    };
    Common::DynArray<DescriptorInfo> mDescriptors;

    void DeleteDescriptors();
};

} // namespace Renderer
} // namespace NFE
