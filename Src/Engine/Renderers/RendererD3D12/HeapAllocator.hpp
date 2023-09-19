/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Heap allocator declarations
 */

#pragma once

#include "Common.hpp"
#include "Descriptors.hpp"

#include "Engine/Common/Containers/DynArray.hpp"
#include "Engine/Common/System/RWLock.hpp"


namespace NFE {
namespace Renderer {

class HeapAllocator
{
public:

    enum class Type : uint8
    {
        CbvSrvUav,
        Sampler,
        Rtv,
        Dsv,
    };

private:
    Type mType;
    uint32 mSize;
    bool mShaderVisible;

    Common::RWLock mLock;

    Common::DynArray<bool> mBitmap;

    D3DPtr<ID3D12DescriptorHeap> mHeap;

    D3D12_CPU_DESCRIPTOR_HANDLE mCpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE mGpuHandle;
    UINT mDescriptorSize;

    HeapAllocator(const HeapAllocator&) = delete;
    HeapAllocator& operator=(const HeapAllocator&) = delete;

public:
    HeapAllocator(Type type, uint32 initialSize, bool shaderVisible = false);
    ~HeapAllocator();

    bool Init();
    void Release();

    DescriptorID Allocate(uint32 numDescriptors);

    void Free(DescriptorID startDescriptor, uint32 numDescriptors);

    NFE_INLINE ID3D12DescriptorHeap* GetHeap() const
    {
        return mHeap.Get();
    }

    NFE_INLINE const D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(uint32 offset = 0) const
    {
        NFE_ASSERT(offset < mSize, "Descriptor index (%u) out of heap bounds (size = %u)", offset, mSize);

        D3D12_CPU_DESCRIPTOR_HANDLE handle = mCpuHandle;
        handle.ptr += mDescriptorSize * offset;
        return handle;
    }

    NFE_INLINE const D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(uint32 offset = 0) const
    {
        NFE_ASSERT(offset < mSize, "Descriptor index (%u) out of heap bounds (size = %u)", offset, mSize);

        D3D12_GPU_DESCRIPTOR_HANDLE handle = mGpuHandle;
        handle.ptr += mDescriptorSize * offset;
        return handle;
    }

    NFE_INLINE UINT GetDescriptorSize() const
    {
        return mDescriptorSize;
    }
};

} // namespace Renderer
} // namespace NFE
