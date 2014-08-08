#include "stdafx.h"
#include "DebugRenderer.h"
#include "Renderer.h"

namespace NFE {
namespace Render {

using namespace Math;

DebugRendererD3D11::DebugRendererD3D11()
{
    mDepthStencilState = 0;
    mRasterizerState = 0;
    mVertexBuffer = 0;
    mIndexBuffer = 0;
    mInputLayout = 0;
    mMeshInputLayout = 0;
    mCBuffer = 0;
    mSampler = 0;

    mTexture = 0;
    mVertexQueue = 0;
    mIndexBuffer = 0;

    mPolyType = DebugPolyType::Line;
}

DebugRendererD3D11::~DebugRendererD3D11()
{
    //TODO: check if module has been released
}

Result DebugRendererD3D11::Init(RendererD3D11* pRenderer)
{
    HRESULT HR = 0;

    //compile shaders
    mShaderVS.Init(pRenderer, ShaderType::Vertex, "DebugVS");

    mShaderPS.AddMacro("USE_TEXTURE", 1, 0);
    mShaderPS.Init(pRenderer, ShaderType::Pixel, "DebugPS");

    mMeshShaderVS.Init(pRenderer, ShaderType::Vertex, "MeshDebugVS");

    mMeshShaderPS.AddMacro("USE_TEXTURE", 1, 0);
    mMeshShaderPS.Init(pRenderer, ShaderType::Pixel, "MeshDebugPS");



    D3D11_BUFFER_DESC bd;
    bd.MiscFlags = 0;

    //init Constant Buffer
    bd.ByteWidth = sizeof(DebugCBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.StructureByteStride = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    HR |= D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mCBuffer));


    const D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",      0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    //init Input Layout
    Common::Buffer* pShaderSource = mShaderVS.GetShaderBytecode(0);
    if (pShaderSource)
        D3D_CHECK(pRenderer->D3DDevice->CreateInputLayout(layout, 3, pShaderSource->GetData(),
                  pShaderSource->GetSize(), &mInputLayout));


    const D3D11_INPUT_ELEMENT_DESC meshLayout[] =
    {
        { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",    0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,        0,  36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    //init mesh Input Layout
    pShaderSource = mMeshShaderVS.GetShaderBytecode(0);
    if (pShaderSource)
        D3D_CHECK(pRenderer->D3DDevice->CreateInputLayout(meshLayout, 4, pShaderSource->GetData(),
                  pShaderSource->GetSize(), &mMeshInputLayout));



    //init Vertex Buffer
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.ByteWidth = DEBUG_VB_SIZE * sizeof(DebugVertex);
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.StructureByteStride = sizeof(DebugVertex);
    HR |= D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mVertexBuffer));

    //init Index Buffer
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.ByteWidth = DEBUG_IB_SIZE * sizeof(int);
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.StructureByteStride = 0;
    HR |= D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mIndexBuffer));

    //alloc queue
    mVertexQueue = (DebugVertex*)malloc(DEBUG_VB_SIZE * sizeof(DebugVertex));
    mVertexQueueSize = 0;
    mIndexQueue = (UINT*)malloc(DEBUG_IB_SIZE * sizeof(UINT));
    mIndexQueueSize = 0;

    RendererD3D11SamplerDesc samplerDesc;
    D3D_CHECK(pRenderer->D3DDevice->CreateSamplerState(&samplerDesc, &mSampler));

    // Initialize depth stencil state
    D3D11_RASTERIZER_DESC rd;
    ZeroMemory(&rd, sizeof(rd));
    rd.CullMode = D3D11_CULL_BACK;
    rd.FrontCounterClockwise = 1;
    rd.FillMode = D3D11_FILL_SOLID;
    rd.DepthClipEnable = 1;
    D3D_CHECK(pRenderer->D3DDevice->CreateRasterizerState(&rd, &mRasterizerState));


    // Initialize depth stencil state
    RendererD3D11DepthStencilDesc depthStencilDesc(TRUE, D3D11_DEPTH_WRITE_MASK_ALL,
            D3D11_COMPARISON_LESS);
    D3D_CHECK(pRenderer->D3DDevice->CreateDepthStencilState(&depthStencilDesc,
              &mDepthStencilState));

    return Result::OK;
}

void DebugRendererD3D11::Release()
{
    D3D_SAFE_RELEASE(mDepthStencilState);
    D3D_SAFE_RELEASE(mRasterizerState);
    D3D_SAFE_RELEASE(mVertexBuffer);
    D3D_SAFE_RELEASE(mIndexBuffer);
    D3D_SAFE_RELEASE(mInputLayout);
    D3D_SAFE_RELEASE(mMeshInputLayout);
    D3D_SAFE_RELEASE(mCBuffer);
    D3D_SAFE_RELEASE(mSampler);

    mShaderVS.Release();
    mShaderPS.Release();
    mMeshShaderVS.Release();
    mMeshShaderPS.Release();

    if (mVertexQueue)
    {
        free(mVertexQueue);
        mVertexQueue = 0;
    }

    if (mIndexQueue)
    {
        free(mIndexQueue);
        mIndexQueue = 0;
    }
}

void DebugRendererD3D11::Enter(NFE_CONTEXT_ARG)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    auto pRenderer = pCtx->GetRenderer();
    //mRenderer->bindShader(&mShaderVS, 0);
    //mRenderer->bindShader(&mShaderPS, 0);

    mPolyType = DebugPolyType::Line;
    mTexture = 0;

    UINT strides[] = {sizeof(DebugVertex)};
    UINT offsets[] = {0};
    pCtx->D3DContext->IASetVertexBuffers(0, 1, &mVertexBuffer, strides, offsets);
    pCtx->D3DContext->IASetInputLayout(mInputLayout);
    pCtx->D3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    pCtx->D3DContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    pCtx->D3DContext->VSSetConstantBuffers(0, 1, &mCBuffer);

    pCtx->D3DContext->OMSetRenderTargets(1, &(pRenderer->renderTarget.RTV), 0);
    pCtx->D3DContext->OMSetDepthStencilState(mDepthStencilState, 0);
    pCtx->D3DContext->RSSetState(mRasterizerState);
    pCtx->D3DContext->PSSetSamplers(0, 1, &mSampler);
}

void DebugRendererD3D11::Leave(NFE_CONTEXT_ARG)
{
    Flush(pContext);
}

void DebugRendererD3D11::SetTarget(NFE_CONTEXT_ARG, IRenderTarget* pTarget)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    RenderTargetD3D11* pRT = dynamic_cast<RenderTargetD3D11*>(pTarget);

    if (pTarget)
        pCtx->D3DContext->OMSetRenderTargets(1, &pRT->RTV, 0);
}

void DebugRendererD3D11::SetCamera(NFE_CONTEXT_ARG, const Matrix& viewMatrix,
                                   const Matrix& projMatrix)
{
    auto pCtx = (RenderContextD3D11*)pContext;

    DebugCBuffer debugCBufferData;
    debugCBufferData.projMatrix = projMatrix;
    debugCBufferData.viewMatrix = viewMatrix;
    pCtx->D3DContext->UpdateSubresource(mCBuffer, 0, 0, &debugCBufferData, 0, 0);
}


void DebugRendererD3D11::Flush(NFE_CONTEXT_ARG)
{
    auto pCtx = (RenderContextD3D11*)pContext;

    if (mVertexQueueSize == 0 && mIndexQueueSize == 0)
        return;

    D3D11_MAPPED_SUBRESOURCE mappedVB, mappedIB;

    //update VB
    pCtx->D3DContext->Map(mVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVB);
    memcpy(mappedVB.pData, mVertexQueue, sizeof(DebugVertex) * mVertexQueueSize);
    pCtx->D3DContext->Unmap(mVertexBuffer, 0);

    //update IB
    pCtx->D3DContext->Map(mIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedIB);
    memcpy(mappedIB.pData, mIndexQueue, sizeof(UINT) * mIndexQueueSize);
    pCtx->D3DContext->Unmap(mIndexBuffer, 0);


    UINT strides[] = {sizeof(DebugVertex)};
    UINT offsets[] = {0};
    pCtx->D3DContext->IASetVertexBuffers(0, 1, &mVertexBuffer, strides, offsets);
    pCtx->D3DContext->IASetInputLayout(mInputLayout);
    pCtx->D3DContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);


    D3D11_PRIMITIVE_TOPOLOGY primTopology;
    UINT useTexture = 0;

    switch (mPolyType)
    {
        case DebugPolyType::Point:
            primTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
            useTexture = 0;
            break;

        case DebugPolyType::Line:
            primTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
            useTexture = 0;
            break;

        case DebugPolyType::Triangle:
            primTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            useTexture = 0;
            break;

        case DebugPolyType::TriangleTex:
            primTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            useTexture = 1;
            break;
    }

    if (mTexture == 0)
        useTexture = 0;

    pCtx->BindShader(&mShaderVS, &useTexture);
    pCtx->BindShader(&mShaderPS, &useTexture);

    if (useTexture)
    {
        pCtx->D3DContext->PSSetShaderResources(0, 1, &mTexture->SRV);
    }

    pCtx->D3DContext->IASetPrimitiveTopology(primTopology);
    pCtx->D3DContext->DrawIndexed(mIndexQueueSize, 0, 0);

    mVertexQueueSize = 0;
    mIndexQueueSize = 0;
}

void DebugRendererD3D11::DrawLine(NFE_CONTEXT_ARG, const Vector& A, const Vector& B,
                                  const UINT color)
{
    if (mPolyType != DebugPolyType::Line)
    {
        Flush(pContext);
        mPolyType = DebugPolyType::Line;
    }

    if ((mIndexQueueSize >= DEBUG_IB_SIZE - 2) || (mVertexQueueSize >= DEBUG_VB_SIZE - 2))
        Flush(pContext);

    DebugVertex vert;
    vert.color = color;
    vert.texCoord = Float2();

    VectorStore(A, &vert.pos);
    mIndexQueue[mIndexQueueSize++] = mVertexQueueSize;
    mVertexQueue[mVertexQueueSize++] = vert;


    VectorStore(B, &vert.pos);
    mIndexQueue[mIndexQueueSize++] = mVertexQueueSize;
    mVertexQueue[mVertexQueueSize++] = vert;
}

void DebugRendererD3D11::DrawLine(NFE_CONTEXT_ARG, const Float3& A, const Float3& B,
                                  const UINT color)
{
    if (mPolyType != DebugPolyType::Line)
    {
        Flush(pContext);
        mPolyType = DebugPolyType::Line;
    }

    if ((mIndexQueueSize >= DEBUG_IB_SIZE - 2) || (mVertexQueueSize >= DEBUG_VB_SIZE - 2))
        Flush(pContext);

    DebugVertex vert;
    vert.color = color;

    vert.pos = A;
    mIndexQueue[mIndexQueueSize++] = mVertexQueueSize;
    mVertexQueue[mVertexQueueSize++] = vert;

    vert.pos = B;
    mIndexQueue[mIndexQueueSize++] = mVertexQueueSize;
    mVertexQueue[mVertexQueueSize++] = vert;
}

static const UINT g_BoxIndexBuffer[] = {0, 1, 1, 3, 3, 2, 2, 0,
                                        4, 5, 5, 7, 7, 6, 6, 4,
                                        0, 4, 1, 5, 2, 6, 3, 7
                                       };

void DebugRendererD3D11::DrawBox(NFE_CONTEXT_ARG, const Box& box, const UINT color)
{
    if (mPolyType != DebugPolyType::Line)
    {
        Flush(pContext);
        mPolyType = DebugPolyType::Line;
    }

    if ((mIndexQueueSize >= DEBUG_IB_SIZE - 24) || (mVertexQueueSize >= DEBUG_VB_SIZE - 8))
        Flush(pContext);

    DebugVertex boxVerticies[8];
    for (int i = 0; i < 8; i++)
    {
        boxVerticies[i].pos.x = (i & (1 << 0)) ? (box.max.f[0]) : (box.min.f[0]);
        boxVerticies[i].pos.y = (i & (1 << 1)) ? (box.max.f[1]) : (box.min.f[1]);
        boxVerticies[i].pos.z = (i & (1 << 2)) ? (box.max.f[2]) : (box.min.f[2]);
        boxVerticies[i].color = color;
    }
    memcpy(mVertexQueue + mVertexQueueSize, boxVerticies, 8 * sizeof(DebugVertex));

    for (int i = 0; i < 24; i++)
        mIndexQueue[mIndexQueueSize + i] = g_BoxIndexBuffer[i] + mVertexQueueSize;

    mVertexQueueSize += 8;
    mIndexQueueSize += 24;
}


static const UINT g_FilledBoxIndexBuffer[] = {0, 1, 3,  0, 3, 2,
                                              1, 5, 7,  1, 7, 3,
                                              5, 6, 4,  5, 7, 6,
                                              4, 2, 0,  4, 6, 2,
                                              2, 3, 7,  2, 7, 6,
                                              4, 1, 0,  4, 5, 1
                                             };

void DebugRendererD3D11::DrawFilledBox(NFE_CONTEXT_ARG, const Box& box, const UINT color)
{
    if (mPolyType != DebugPolyType::Triangle)
    {
        Flush(pContext);
        mPolyType = DebugPolyType::Triangle;
    }

    if ((mIndexQueueSize >= DEBUG_IB_SIZE - 36) || (mVertexQueueSize >= DEBUG_VB_SIZE - 8))
        Flush(pContext);

    DebugVertex boxVerticies[8];
    for (int i = 0; i < 8; i++)
    {
        boxVerticies[i].pos.x = (i & (1 << 0)) ? (box.max.f[0]) : (box.min.f[0]);
        boxVerticies[i].pos.y = (i & (1 << 1)) ? (box.max.f[1]) : (box.min.f[1]);
        boxVerticies[i].pos.z = (i & (1 << 2)) ? (box.max.f[2]) : (box.min.f[2]);
        boxVerticies[i].color = color;
    }
    memcpy(mVertexQueue + mVertexQueueSize, boxVerticies, 8 * sizeof(DebugVertex));

    for (int i = 0; i < 36; i++)
        mIndexQueue[mIndexQueueSize + i] = g_FilledBoxIndexBuffer[i] + mVertexQueueSize;

    mVertexQueueSize += 8;
    mIndexQueueSize += 36;
}

void DebugRendererD3D11::DrawFrustum(NFE_CONTEXT_ARG, const Frustum& frustum, const UINT color)
{
    if (mPolyType != DebugPolyType::Line)
    {
        Flush(pContext);
        mPolyType = DebugPolyType::Line;
    }

    if ((mIndexQueueSize >= DEBUG_IB_SIZE - 24) || (mVertexQueueSize >= DEBUG_VB_SIZE - 8))
        Flush(pContext);

    DebugVertex boxVerticies[8];
    for (int i = 0; i < 8; i++)
    {
        VectorStore(frustum.verticies[i], &boxVerticies[i].pos);
        boxVerticies[i].color = color;
    }
    memcpy(mVertexQueue + mVertexQueueSize, boxVerticies, 8 * sizeof(DebugVertex));

    for (int i = 0; i < 24; i++)
        mIndexQueue[mIndexQueueSize + i] = g_BoxIndexBuffer[i] + mVertexQueueSize;

    mVertexQueueSize += 8;
    mIndexQueueSize += 24;
}

void DebugRendererD3D11::DrawQuad(NFE_CONTEXT_ARG, const Vector& pos, IRendererTexture* pTexture,
                                  const UINT color)
{
    RendererTextureD3D11* pTex = dynamic_cast<RendererTextureD3D11*>(pTexture);

    if ((mPolyType != DebugPolyType::TriangleTex) || (mTexture != pTex))
    {
        Flush(pContext);
        mPolyType = DebugPolyType::TriangleTex;
        mTexture = pTex;
    }


    if ((mIndexQueueSize >= DEBUG_IB_SIZE - 6) || (mVertexQueueSize >= DEBUG_VB_SIZE - 6))
        Flush(pContext);


    DebugVertex vert[4];
    vert[0].color = color;
    vert[0].pos = Float3(pos.f[0] - 1.0f, pos.f[1], pos.f[2] + 1.0f);
    vert[0].texCoord = Float2(0.0f, 0.0f);

    vert[1].color = color;
    vert[1].pos = Float3(pos.f[0] + 1.0f, pos.f[1], pos.f[2] + 1.0f);
    vert[1].texCoord = Float2(1.0f, 0.0f);

    vert[2].color = color;
    vert[2].pos = Float3(pos.f[0] + 1.0f, pos.f[1], pos.f[2] - 1.0f);
    vert[2].texCoord = Float2(1.0f, 1.0f);

    vert[3].color = color;
    vert[3].pos = Float3(pos.f[0] - 1.0f, pos.f[1], pos.f[2] - 1.0f);
    vert[3].texCoord = Float2(0.0f, 1.0f);


    mVertexQueue[mVertexQueueSize  ] = vert[0];
    mVertexQueue[mVertexQueueSize + 1] = vert[1];
    mVertexQueue[mVertexQueueSize + 2] = vert[2];
    mVertexQueue[mVertexQueueSize + 3] = vert[3];


    mIndexQueue[mIndexQueueSize  ] = mVertexQueueSize;
    mIndexQueue[mIndexQueueSize + 1] = mVertexQueueSize + 1;
    mIndexQueue[mIndexQueueSize + 2] = mVertexQueueSize + 2;

    mIndexQueue[mIndexQueueSize + 3] = mVertexQueueSize + 0;
    mIndexQueue[mIndexQueueSize + 4] = mVertexQueueSize + 2;
    mIndexQueue[mIndexQueueSize + 5] = mVertexQueueSize + 3;


    mVertexQueueSize += 4;
    mIndexQueueSize += 6;
}


// Not used now, could be restored in the future
/*
void DebugRendererD3D11::SetMaterial(NFE_CONTEXT_ARG, const Material* pMaterial)
{
    Texture* pTexture = 0;

    if (pMaterial)
        if (pMaterial->mLayers)
            pTexture = pMaterial->mLayers[0].mDiffuseTexture;


    RendererTextureD3D11* pTex = mTexture ? dynamic_cast<RendererTextureD3D11*>(pTexture->GetRendererTexture()) : NULL;
    UINT useTexture = pTex ? 1 : 0;

    pCtx->BindShader(&mMeshShaderVS, &useTexture);
    pCtx->BindShader(&mMeshShaderPS, &useTexture);

    if (useTexture)
    {
        pCtx->D3DContext->PSSetShaderResources(0, 1, &pTex->mSRV);
    }
}
*/

/*
void DebugRendererD3D11::DrawMesh(NFE_CONTEXT_ARG, const Mesh* pMesh)
{
    if (!pMesh)
        return;

    if (pMesh->GetState() != RES_LOADED)
        return;

    auto pVB = (RendererBufferD3D11*)pMesh->mIB;
    auto pIB = (RendererBufferD3D11*)pMesh->mVB;

    if ((pIB->mD3DBuffer == 0) || (pVB->mD3DBuffer == 0))
        return;


    Flush(pCtx);

    if (mPolyType != X_MESH)
    {
        pCtx->D3DContext->IASetInputLayout(mMeshInputLayout);
        pCtx->D3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        mPolyType = X_MESH;
    }


    UINT strides[] = {sizeof(MeshVertex)};
    UINT offsets[] = {0};
    pCtx->D3DContext->IASetVertexBuffers(0, 1, &pVB->mD3DBuffer, strides, offsets);
    pCtx->D3DContext->IASetIndexBuffer(pIB->mD3DBuffer, DXGI_FORMAT_R32_UINT, 0);

    for (uint32 i = 0; i<pMesh->mSubMeshesCount; i++)
    {
        SetMaterial(pCtx, pMesh->mSubMeshes[i].pMaterial);
        pCtx->D3DContext->DrawIndexed(3*pMesh->mSubMeshes[i].mTrianglesCount,
                                      pMesh->mSubMeshes[i].mIndexOffset, 0);
    }
}
*/

} // namespace Render
} // namespace NFE
