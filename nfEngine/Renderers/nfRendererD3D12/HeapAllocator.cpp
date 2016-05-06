/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Heap allocator implementation
 */

#include "PCH.hpp"
#include "HeapAllocator.hpp"
#include "RendererD3D12.hpp"
#include "nfCommon/Logger.hpp"


namespace NFE {
namespace Renderer {


namespace {

const char* GetHeapName(HeapAllocator::Type type)
{
    switch (type)
    {
    case HeapAllocator::Type::CbvSrvUav:
        return "CBV_SRV_UAV";
    case HeapAllocator::Type::Rtv:
        return "RTV";
    case HeapAllocator::Type::Dsv:
        return "DSV";
    }
    return "(Unknown)";
}

} // namespace

HeapAllocator::HeapAllocator(Type type, uint32 initialSize)
    : mType(type)
    , mSize(initialSize)
{
    mBitmap.resize(initialSize);
    for (uint32 i = 0; i < initialSize; ++i)
        mBitmap[i] = false;
}

HeapAllocator::~HeapAllocator()
{
    uint32 allocatedDescriptors = 0;
    for (bool state : mBitmap)
        if (state)
            allocatedDescriptors++;

    if (allocatedDescriptors > 0)
        LOG_WARNING("There are still %u descriptors allocated in %s heap",
                    allocatedDescriptors, GetHeapName(mType));
}

bool HeapAllocator::Init()
{
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = static_cast<UINT>(mBitmap.size());
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    switch (mType)
    {
    case Type::CbvSrvUav:
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heapDesc.Flags |= D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        break;
    case Type::Rtv:
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        break;
    case Type::Dsv:
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        break;
    }

    HRESULT hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mHeap)));
    if (FAILED(hr))
        return false;

    mCpuHandle = mHeap->GetCPUDescriptorHandleForHeapStart();
    mGpuHandle = mHeap->GetGPUDescriptorHandleForHeapStart();
    mDescriptorSize = gDevice->GetDevice()->GetDescriptorHandleIncrementSize(heapDesc.Type);
    LOG_DEBUG("%s descriptor heap handle increment: %u", GetHeapName(mType), mDescriptorSize);

    return true;
}

uint32 HeapAllocator::Allocate(uint32 numDescriptors)
{
    // TODO this allocator is extremly slow

    uint32 first = 0;
    uint32 count = 0;
    for (uint32 i = 0; i < mSize; ++i)
    {
        if (mBitmap[i])
        {
            count = 0;
            first = i + 1;
            continue;
        }

        count++;
        if (count >= numDescriptors)
        {
            for (size_t j = first; j < first + count; ++j)
                mBitmap[i] = true;
            return first;
        }
    }

    LOG_ERROR("Descriptor heap allocation failed");
    return UINT32_MAX;
}

void HeapAllocator::Free(uint32 offset, uint32 numDescriptors)
{
    assert(offset + numDescriptors <= mSize);

    for (uint32 i = offset; i < offset + numDescriptors; ++i)
    {
        assert(mBitmap[i]);
        mBitmap[i] = false;
    }
}

} // namespace Renderer
} // namespace NFE
