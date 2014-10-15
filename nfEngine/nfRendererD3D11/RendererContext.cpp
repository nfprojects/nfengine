#include "stdafx.hpp"
#include "RendererContext.hpp"
#include "Renderer.hpp"

namespace NFE {
namespace Render {

RenderContextD3D11::RenderContextD3D11()
{
    mBoundVS = 0;
    mBoundGS = 0;
    mBoundDS = 0;
    mBoundHS = 0;
    mBoundPS = 0;
    mBoundCS = 0;

    D3DContext = 0;
    commandList = 0;

    instanceData = (InstanceData*)_aligned_malloc(MAX_BUFFERED_INSTANCES * sizeof(InstanceData), 16);
}

RenderContextD3D11::~RenderContextD3D11()
{
    D3D_SAFE_RELEASE(D3DContext);

    _aligned_free(instanceData);
    instanceData = 0;
}

void RenderContextD3D11::Init(uint32 id, RendererD3D11* pRenderer, ID3D11DeviceContext* pContext)
{
    NFE_ASSERT(pRenderer != NULL, L"pRenderer is null");
    mRenderer = pRenderer;

    D3D_SAFE_RELEASE(commandList);
    D3D_SAFE_RELEASE(D3DContext);

    if (pContext)
    {
        D3DContext = pContext;
    }
    else
    {
        D3DContext = 0;
        D3D_CHECK(pRenderer->D3DDevice->CreateDeferredContext(0, &D3DContext));
    }
}

void RenderContextD3D11::Release()
{
    D3D_SAFE_RELEASE(commandList);
    D3D_SAFE_RELEASE(D3DContext);

    if (instanceData)
    {
        _aligned_free(instanceData);
        instanceData = 0;
    }
}

void RenderContextD3D11::Begin()
{
    mBoundVS = 0;
    mBoundDS = 0;
    mBoundHS = 0;
    mBoundPS = 0;
    mBoundCS = 0;
    mBoundGS = 0;

    D3D_SAFE_RELEASE(commandList);
}

void RenderContextD3D11::End()
{
    D3DContext->FinishCommandList(0, &commandList);
    //commandList->
}


int RenderContextD3D11::BindShader(Multishader* pShader, const UINT* pMacroValues)
{
    //calculate hash value
    UINT Hash = 0;
    if (pMacroValues)
    {
        for (uint32 i = 0; i < pShader->macros.size(); i++)
            Hash |= (pMacroValues[i] << pShader->macros[i].bits);
    }

    SubshaderInfo& shaderInfo = pShader->shaders[Hash];

    switch (pShader->GetType())
    {
        case ShaderType::Vertex:
        {
            if (mBoundVS != (ID3D11VertexShader*)shaderInfo.shader)
            {
                mBoundVS = (ID3D11VertexShader*)shaderInfo.shader;
                D3DContext->VSSetShader(mBoundVS, 0, 0);
            }
            break;
        }

        case ShaderType::Pixel:
        {
            if (mBoundPS != (ID3D11PixelShader*)shaderInfo.shader)
            {
                mBoundPS = (ID3D11PixelShader*)shaderInfo.shader;
                D3DContext->PSSetShader(mBoundPS, 0, 0);
            }
            break;
        }

        case ShaderType::Domain:
        {
            if (mBoundDS != (ID3D11DomainShader*)shaderInfo.shader)
            {
                mBoundDS = (ID3D11DomainShader*)shaderInfo.shader;
                D3DContext->DSSetShader(mBoundDS, 0, 0);
            }
            break;
        }

        case ShaderType::Hull:
        {
            if (mBoundHS != (ID3D11HullShader*)shaderInfo.shader)
            {
                mBoundHS = (ID3D11HullShader*)shaderInfo.shader;
                D3DContext->HSSetShader(mBoundHS, 0, 0);
            }
            break;
        }

        case ShaderType::Geometry:
        {
            if (mBoundGS != (ID3D11GeometryShader*)shaderInfo.shader)
            {
                mBoundGS = (ID3D11GeometryShader*)shaderInfo.shader;
                D3DContext->GSSetShader(mBoundGS, 0, 0);
            }
            break;
        }

        case ShaderType::Compute:
        {
            if (mBoundCS != (ID3D11ComputeShader*)shaderInfo.shader)
            {
                mBoundCS = (ID3D11ComputeShader*)shaderInfo.shader;
                D3DContext->CSSetShader(mBoundCS, 0, 0);
            }
            break;
        }

        default:
            return 1;
    }

    return 0;
}

int RenderContextD3D11::ResetShader(ShaderType type)
{
    switch (type)
    {
        case ShaderType::Vertex:
            if (mBoundVS != 0)
            {
                mBoundVS = 0;
                D3DContext->VSSetShader(0, 0, 0);
            }
            return 0;

        case ShaderType::Pixel:
            if (mBoundPS != 0)
            {
                mBoundPS = 0;
                D3DContext->PSSetShader(0, 0, 0);
            }
            return 0;

        case ShaderType::Geometry:
            if (mBoundGS != 0)
            {
                mBoundGS = 0;
                D3DContext->GSSetShader(0, 0, 0);
            }
            return 0;

        case ShaderType::Domain:
            if (mBoundDS != 0)
            {
                mBoundDS = 0;
                D3DContext->DSSetShader(0, 0, 0);
            }
            return 0;

        case ShaderType::Hull:
            if (mBoundHS != 0)
            {
                mBoundHS = 0;
                D3DContext->HSSetShader(0, 0, 0);
            }
            return 0;

        case ShaderType::Compute:
            if (mBoundCS != 0)
            {
                mBoundCS = 0;
                D3DContext->CSSetShader(0, 0, 0);
            }
            return 0;
    }

    return 1;
}


RendererD3D11* RenderContextD3D11::GetRenderer() const
{
    return mRenderer;
}

bool RenderContextD3D11::Execute(IRenderContext* pContext, bool saveState)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    if (pCtx->commandList)
    {
        pCtx->D3DContext->ExecuteCommandList(pCtx->commandList, saveState);
        D3D_SAFE_RELEASE(pCtx->commandList);

        //copy states
        if (!saveState)
        {
            mBoundVS = pCtx->mBoundVS;
            mBoundPS = pCtx->mBoundPS;
            mBoundGS = pCtx->mBoundGS;
            mBoundDS = pCtx->mBoundDS;
            mBoundHS = pCtx->mBoundHS;
            mBoundCS = pCtx->mBoundCS;
        }

        return true;
    }

    return false;
}

} // namespace Render
} // namespace NFE
