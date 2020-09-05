/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Heap allocator declarations
 */

#pragma once

#include "Common.hpp"

#include "Engine/Common/Containers/DynArray.hpp"
#include "Engine/Common/System/RWLock.hpp"


namespace NFE {
namespace Renderer {

class HeapAllocator
{
public:
    struct DescriptorRange
    {
        uint32 offset;
        uint32 size;
    };

    enum class Type
    {
        CbvSrvUav,
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

    DescriptorRange Allocate(uint32 numDescriptors);

    void Free(const DescriptorRange& range);

    NFE_INLINE ID3D12DescriptorHeap* GetHeap() const
    {
        return mHeap.Get();
    }

    NFE_INLINE const D3D12_CPU_DESCRIPTOR_HANDLE& GetCpuHandle() const
    {
        return mCpuHandle;
    }

    NFE_INLINE const D3D12_GPU_DESCRIPTOR_HANDLE& GetGpuHandle() const
    {
        return mGpuHandle;
    }

    NFE_INLINE UINT GetDescriptorSize() const
    {
        return mDescriptorSize;
    }
};

} // namespace Renderer
} // namespace NFE
