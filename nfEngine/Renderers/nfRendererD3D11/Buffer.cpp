/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's buffer
 */

#include "PCH.hpp"
#include "Buffer.hpp"
#include "RendererD3D11.hpp"

#include "nfCommon/Logger.hpp"


namespace NFE {
namespace Renderer {

Buffer::Buffer()
{
}

bool Buffer::Init(const BufferDesc& desc)
{
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.ByteWidth = static_cast<UINT>(desc.size);
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    switch (desc.type)
    {
        case BufferType::Vertex:
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            break;
        case BufferType::Index:
            bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            break;
        case BufferType::Constant:
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            break;
        default:
            LOG_ERROR("Invalid buffer type");
            return false;
    }

    switch (desc.mode)
    {
        case BufferMode::Static:
            bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
            break;
        case BufferMode::GPUOnly:
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            break;
        case BufferMode::Dynamic:
        case BufferMode::Volatile:
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            bufferDesc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
            break;
        case BufferMode::Readback:
            bufferDesc.Usage = D3D11_USAGE_STAGING;
            bufferDesc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
            break;
        default:
            LOG_ERROR("Invalid buffer access mode");
            return false;
    }

    HRESULT hr;
    if (desc.initialData)
    {
        D3D11_SUBRESOURCE_DATA initialData;
        initialData.pSysMem = desc.initialData;
        initialData.SysMemPitch = 0;
        initialData.SysMemSlicePitch = 0;
        hr = D3D_CALL_CHECK(gDevice->Get()->CreateBuffer(&bufferDesc, &initialData, &mBuffer));
    }
    else
        hr = D3D_CALL_CHECK(gDevice->Get()->CreateBuffer(&bufferDesc, NULL, &mBuffer));

    if (FAILED(hr))
        return false;

    if (gDevice->IsDebugLayerEnabled())
    {
        /// set debug name
        std::string bufferName = "NFE::Renderer::Buffer \"";
        if (desc.debugName)
            bufferName += desc.debugName;
        bufferName += '"';
        D3D_CALL_CHECK(mBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
                                               static_cast<UINT>(bufferName.length()), bufferName.c_str()));
    }

    mMode = desc.mode;
    mType = desc.type;
    mSize = desc.size;
    return true;
}

} // namespace Renderer
} // namespace NFE
