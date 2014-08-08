#include "stdafx.h"
#include "ShaderResource.h"
#include "Globals.h"
#include "RendererD3D11.h"

bool ShaderCBuffer::Init(uint32 reg, ShaderType shaderType, void* ptr, size_t size,
                         bool updateOnBind)
{
    XRendererD3D11* pRenderer = (XRendererD3D11*)g_pRenderer;

    Release();
    if (ptr == nullptr || size == 0 || shaderType == UNKNOWN_SHADER)
        return false;
    m_register = reg;
    m_shaderType = shaderType;
    m_ptr = ptr;
    m_size = size;
    m_updateOnBind = updateOnBind;

    D3D11_BUFFER_DESC bd;
    bd.MiscFlags = 0;
    bd.ByteWidth = size;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.StructureByteStride = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;

    HRESULT hr = D3D_CHECK(pRenderer->m_pD3DDevice->CreateBuffer(&bd, 0, &m_buffer));

    if (FAILED(hr))
    {
        Release();
        return false;
    }

    m_isValid = true;
    return true;
}

void ShaderCBuffer::Release()
{
    m_isValid = false;
    m_register = 0;
    m_shaderType = UNKNOWN_SHADER;
    m_ptr = 0;
    m_size = 0;
    m_updateOnBind = false;
    D3D_SAFE_RELEASE(m_buffer);
}

void ShaderCBuffer::Update()
{
    if (!m_isValid) return;
    g_pImmediateContext->m_pD3DContext->UpdateSubresource(m_buffer, 0, 0, m_ptr, 0, 0);
}
void ShaderCBuffer::Bind(IRenderContext* pContext)
{
    if (!m_isValid || pContext == nullptr) return;
    if (m_updateOnBind) Update();
    m_lastContext = pContext;

    if (m_shaderType == VERTEX_SHADER)
    {
        m_lastContext->m_pD3DContext->VSSetConstantBuffers(m_register, 1, &m_buffer);
        return;
    }

    if (m_shaderType == PIXEL_SHADER)
    {
        m_lastContext->m_pD3DContext->PSSetConstantBuffers(m_register, 1, &m_buffer);
        return;
    }

    if (m_shaderType == DOMAIN_SHADER)
    {
        m_lastContext->m_pD3DContext->DSSetConstantBuffers(m_register, 1, &m_buffer);
        return;
    }

    if (m_shaderType == HULL_SHADER)
    {
        m_lastContext->m_pD3DContext->HSSetConstantBuffers(m_register, 1, &m_buffer);
        return;
    }

    if (m_shaderType == GEOMETRY_SHADER)
    {
        m_lastContext->m_pD3DContext->GSSetConstantBuffers(m_register, 1, &m_buffer);
        return;
    }

    if (m_shaderType == COMPUTE_SHADER)
    {
        m_lastContext->m_pD3DContext->CSSetConstantBuffers(m_register, 1, &m_buffer);
        return;
    }
}
void ShaderCBuffer::Unbind()
{
    if (!m_isValid || m_lastContext == nullptr) return;
    if (m_shaderType == VERTEX_SHADER)
    {
        m_lastContext->m_pD3DContext->VSSetConstantBuffers(m_register, 1, 0);
        return;
    }

    if (m_shaderType == PIXEL_SHADER)
    {
        m_lastContext->m_pD3DContext->PSSetConstantBuffers(m_register, 1, 0);
        return;
    }

    if (m_shaderType == DOMAIN_SHADER)
    {
        m_lastContext->m_pD3DContext->DSSetConstantBuffers(m_register, 1, 0);
        return;
    }

    if (m_shaderType == HULL_SHADER)
    {
        m_lastContext->m_pD3DContext->HSSetConstantBuffers(m_register, 1, 0);
        return;
    }

    if (m_shaderType == GEOMETRY_SHADER)
    {
        m_lastContext->m_pD3DContext->GSSetConstantBuffers(m_register, 1, 0);
        return;
    }

    if (m_shaderType == COMPUTE_SHADER)
    {
        m_lastContext->m_pD3DContext->CSSetConstantBuffers(m_register, 1, 0);
        return;
    }
    m_lastContext = nullptr;
}