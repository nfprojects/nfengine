#include "stdafx.hpp"
#include "ShadowsRenderer.hpp"
#include "Renderer.hpp"

namespace NFE {
namespace Render {

using namespace Math;

#define INSTANCING (0)

NFE_ALIGN(16)
struct ShadowGlobalCBufferVS
{
    Matrix viewMatrix;
    Matrix projMatrix;
    Matrix viewProjMatrix;
    Vector lightPos;
};

NFE_ALIGN(16)
struct PerInstanceCBufferVS
{
    Matrix worldMatrix;
};

ShadowRendererD3D11::ShadowRendererD3D11()
{
    mDepthStencilState = 0;
    mRasterizerState = 0;
    mInputLayout = 0;
    mSampler = 0;

    mGlobalCBuffer = 0;
    mPerInstanceCBuffer = 0;
    mInstancesVB = 0;
}

ShadowRendererD3D11::~ShadowRendererD3D11()
{
    Release();
}

Result ShadowRendererD3D11::Init(RendererD3D11* pRenderer)
{
    HRESULT HR = 0;

    //compile shaders
    mShaderVS.AddMacro("INSTANCING", 1, 0);
    mShaderVS.Init(pRenderer, ShaderType::Vertex, "ShadowVS");

    mShaderPS.AddMacro("USE_TEXTURE", 1, 0);
    mShaderPS.AddMacro("WRITE_DISTANCE", 1, 1);
    mShaderPS.Init(pRenderer, ShaderType::Pixel, "ShadowPS");



    D3D11_BUFFER_DESC bd;
    bd.MiscFlags = 0;

    //init Constant Buffers
    bd.ByteWidth = sizeof(ShadowGlobalCBufferVS);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.StructureByteStride = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    HR |= D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mGlobalCBuffer));

    bd.ByteWidth = sizeof(PerInstanceCBufferVS);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.StructureByteStride = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    HR |= D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mPerInstanceCBuffer));


    //create instances data vertex buffer
    bd.ByteWidth = MAX_BUFFERED_INSTANCES * sizeof(InstanceData);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.StructureByteStride = 0;
    bd.Usage = D3D11_USAGE_DYNAMIC;
    HR |= D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mInstancesVB));


    //create input layout
    const D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  0,  D3D11_INPUT_PER_VERTEX_DATA,    0},
        { "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,        0,  12, D3D11_INPUT_PER_VERTEX_DATA,    0},
        { "NORMAL",     0, DXGI_FORMAT_R8G8B8A8_SNORM,      0,  20, D3D11_INPUT_PER_VERTEX_DATA,    0},
        { "TANGENT",    0, DXGI_FORMAT_R8G8B8A8_SNORM,      0,  24, D3D11_INPUT_PER_VERTEX_DATA,    0},

        { "TEXCOORD",   1, DXGI_FORMAT_R32G32B32A32_FLOAT,  1,  0,  D3D11_INPUT_PER_INSTANCE_DATA,  1},
        { "TEXCOORD",   2, DXGI_FORMAT_R32G32B32A32_FLOAT,  1,  16, D3D11_INPUT_PER_INSTANCE_DATA,  1},
        { "TEXCOORD",   3, DXGI_FORMAT_R32G32B32A32_FLOAT,  1,  32, D3D11_INPUT_PER_INSTANCE_DATA,  1},
        { "TEXCOORD",   5, DXGI_FORMAT_R32G32B32A32_FLOAT,  1,  48, D3D11_INPUT_PER_INSTANCE_DATA,  1}
    };

    //init Input Layout
    UINT Instancing = INSTANCING;
    Common::Buffer* pShaderSource = mShaderVS.GetShaderBytecode(&Instancing);

#if (INSTANCING > 0)
    D3D_CHECK(pRenderer->D3DDevice->CreateInputLayout(layout, 8, pShaderSource->GetData(),
              pShaderSource->GetSize(), &mInputLayout));
#else
    D3D_CHECK(pRenderer->D3DDevice->CreateInputLayout(layout, 4, pShaderSource->GetData(),
              pShaderSource->GetSize(), &mInputLayout));
#endif

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
    RendererD3D11DepthStencilDesc depthStencilDesc(1, D3D11_DEPTH_WRITE_MASK_ALL,
            D3D11_COMPARISON_LESS);
    D3D_CHECK(pRenderer->D3DDevice->CreateDepthStencilState(&depthStencilDesc,
              &mDepthStencilState));

    return Result::OK;
}

void ShadowRendererD3D11::Release()
{
    D3D_SAFE_RELEASE(mDepthStencilState);
    D3D_SAFE_RELEASE(mRasterizerState);
    D3D_SAFE_RELEASE(mInputLayout);
    D3D_SAFE_RELEASE(mGlobalCBuffer);
    D3D_SAFE_RELEASE(mPerInstanceCBuffer);
    D3D_SAFE_RELEASE(mSampler);

    D3D_SAFE_RELEASE(mInstancesVB);

    mShaderVS.Release();
    mShaderPS.Release();
}

void ShadowRendererD3D11::Enter(NFE_CONTEXT_ARG)
{
    auto pCtx = (RenderContextD3D11*)pContext;

    pCtx->D3DContext->IASetInputLayout(mInputLayout);
    pCtx->D3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D11Buffer* pCBuffers[] = {mGlobalCBuffer, mPerInstanceCBuffer};
    pCtx->D3DContext->VSSetConstantBuffers(0, 2, pCBuffers);
    pCtx->D3DContext->PSSetConstantBuffers(0, 1, pCBuffers);


    pCtx->D3DContext->OMSetDepthStencilState(mDepthStencilState, 0);
    pCtx->D3DContext->RSSetState(mRasterizerState);
    pCtx->D3DContext->PSSetSamplers(0, 1, &mSampler);

    //initialize per instance cbuffer with identity transformation
    //PerInstanceCBufferVS CBufferData;
    //CBufferData.worldMatrix = MatrixIdentity();
    //pCtx->D3DContext->UpdateSubresource(mPerInstanceCBuffer, 0, 0, &CBufferData, 0, 0);

    UINT instancing = INSTANCING;
    pCtx->BindShader(&mShaderVS, &instancing);
    pCtx->BindShader(&mShaderPS, 0);
}

void ShadowRendererD3D11::Leave(NFE_CONTEXT_ARG)
{
    auto pCtx = (RenderContextD3D11*)pContext;

#if (INSTANCING == 1)
    //unbind instance vertex buffers
    ID3D11Buffer* pNullBuffer = 0;
    UINT stride = 0;
    UINT offset = 0;
    pCtx->D3DContext->IASetVertexBuffers(1, 1, &pNullBuffer, &stride, &offset);
#endif
}


void ShadowRendererD3D11::SetDestination(NFE_CONTEXT_ARG, const CameraRenderDesc* pCamera,
        IShadowMap* pShadowMap, uint32 faceID)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    ShadowMapD3D11* pSM = (ShadowMapD3D11*)pShadowMap;

    if (pShadowMap == 0)
        return;

    float clearColor[] = {1.0f, 0.0f, 0.0f, 0.0f};

    D3D11_VIEWPORT viewport;
    viewport.Width = viewport.Height = (float)pShadowMap->GetSize();
    viewport.MaxDepth = 1.0f;
    viewport.MinDepth = 0.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    pCtx->D3DContext->RSSetViewports(1, &viewport);


    //bind shadow map
    ID3D11RenderTargetView* pRTV = pSM->RTVs[faceID];
    pCtx->D3DContext->ClearRenderTargetView(pRTV, clearColor);
    pCtx->D3DContext->ClearDepthStencilView(pSM->DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
    pCtx->D3DContext->OMSetRenderTargets(1, &pRTV, pSM->DSV);

    uint32 macros[2];
    macros[0] = 0;
    macros[1] = (pSM->type == IShadowMap::Type::Cube);
    pCtx->BindShader(&mShaderPS, macros);

    ShadowGlobalCBufferVS CBufferData;
    CBufferData.projMatrix = pCamera->projMatrix;
    CBufferData.viewMatrix = pCamera->viewMatrix;
    CBufferData.viewProjMatrix = pCamera->viewMatrix * pCamera->projMatrix;
    CBufferData.lightPos = pCamera->matrix.r[3];
    pCtx->D3DContext->UpdateSubresource(mGlobalCBuffer, 0, 0, &CBufferData, 0, 0);
}

void ShadowRendererD3D11::SetMaterial(NFE_CONTEXT_ARG, const RendererMaterial* pMaterial)
{
    /*
        TODO: alpha testing
    */

    /*
    Texture* pDiffuseTexture = 0;

    UINT useTexture[2];
    useTexture[0] = 0;
    useTexture[1] = 0;

    if (pMaterial)
        if (pMaterial->m_pLayers)
            pDiffuseTexture = pMaterial->m_pLayers[0].m_pDiffuseTexture;

    if (pDiffuseTexture)
        if (pDiffuseTexture->GetRendererTexture())
            useTexture[0] = 1;

    /*
    pCtx->BindShader(&mShaderPS, useTexture);

    if (useTexture[0])
        pCtx->D3DContext->PSSetShaderResources(0, 1, &pDiffuseTexture->m_pTex->SRV);
    else
        pCtx->D3DContext->PSSetShaderResources(0, 1, &pRenderer->defaultDiffuseTexture->SRV);
    */
}

// TODO: that's almost the same as GBufferRendererD3D11::Draw() - unify
void ShadowRendererD3D11::Draw(NFE_CONTEXT_ARG, const RenderCommandBuffer& buffer)
{
    auto pCtx = (RenderContextD3D11*)pContext;

#if (INSTANCING > 0)

    if (buffer.commands.size() <= 0)
        return;

    //bind instances VB
    UINT strides[] = {sizeof(InstanceData)};
    UINT offsets[] = {0};
    pCtx->D3DContext->IASetVertexBuffers(1, 1, &mInstancesVB, strides, offsets);


    const RendererMaterial* pCurrMaterial = (const RendererMaterial*)(-1);
    IRendererBuffer* pCurrVB = NULL;
    IRendererBuffer* pCurrIB = NULL;
    uint32 currStartIndex = 0xFFFFFFFF;
    uint32 currIndexCount = 0;


    int bufferedInstances = 0;
    uint32 startInstanceLocation = 0;


    for (int i = 0; i < buffer.commands.size(); i++)
    {
        const RenderCommand& command = buffer.commands[i];

        bool bufferIsFull = (bufferedInstances + bufferedInstances >= MAX_BUFFERED_INSTANCES) || (i == 0);
        bool materialChange = (command.pMaterial != pCurrMaterial);
        bool meshChange = ((pCurrIB != command.pIB) || (pCurrVB != command.pVB) ||
                           (currStartIndex != command.startIndex));

        //flush buffered instances
        if (materialChange || meshChange || bufferIsFull)
        {
            if (bufferedInstances > 0)
            {
                pCtx->D3DContext->DrawIndexedInstanced(currIndexCount, bufferedInstances, currStartIndex, 0,
                                                       startInstanceLocation);
                startInstanceLocation += bufferedInstances;
                bufferedInstances = 0;
            }
        }

        //instances buffer is full
        if (bufferIsFull)
        {
            //udapte per instance data buffer
            int instancesToBuffer = Min<int>(MAX_BUFFERED_INSTANCES, buffer.commands.size() - i);

            D3D11_MAPPED_SUBRESOURCE mappedSubres;
            for (int j = 0; j < instancesToBuffer; j++)
            {
                Matrix tmpMatrix = MatrixTranspose(buffer.commands[j + i].matrix);
                pCtx->instanceData[j].worldMatrix[0] = tmpMatrix.r[0];
                pCtx->instanceData[j].worldMatrix[1] = tmpMatrix.r[1];
                pCtx->instanceData[j].worldMatrix[2] = tmpMatrix.r[2];
                pCtx->instanceData[j].velocity = buffer.commands[j + i].velocity;
                pCtx->instanceData[j].angularVelocity = buffer.commands[j + i].angularVelocity;
            }
            pCtx->D3DContext->Map(mInstancesVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubres);
            memcpy(mappedSubres.pData, pCtx->instanceData, instancesToBuffer * sizeof(InstanceData));
            pCtx->D3DContext->Unmap(mInstancesVB, 0);
            bufferedInstances = startInstanceLocation = 0;
        }

        //material has changed
        if (materialChange)
        {
            pCurrMaterial = command.pMaterial;
            SetMaterial(pCtx, pCurrMaterial);
        }

        //mesh has changed
        if (meshChange)
        {
            currStartIndex = command.startIndex;
            currIndexCount = command.indexCount;
            pCurrIB = command.pIB;
            pCurrVB = command.pVB;
            RendererBufferD3D11* pIB = (RendererBufferD3D11*)pCurrIB;
            RendererBufferD3D11* pVB = (RendererBufferD3D11*)pCurrVB;

            UINT stride = sizeof(MeshVertex);
            UINT offset = 0;
            pCtx->D3DContext->IASetVertexBuffers(0, 1, &pVB->D3DBuffer, &stride, &offset);
            pCtx->D3DContext->IASetIndexBuffer(pIB->D3DBuffer, DXGI_FORMAT_R32_UINT, 0);
        }

        bufferedInstances++;
    }


    //flush
    if (bufferedInstances > 0)
        pCtx->D3DContext->DrawIndexedInstanced(currIndexCount, bufferedInstances, currStartIndex, 0,
                                               startInstanceLocation);

#else
    const RendererMaterial* pCurrMaterial = (const RendererMaterial*)(-1);
    IRendererBuffer* pCurrVB = NULL;
    IRendererBuffer* pCurrIB = NULL;
    uint32 currStartIndex = 0xFFFFFFFF;

    for (int i = 0; i < buffer.commands.size(); i++)
    {
        const RenderCommand& command = buffer.commands[i];

        bool materialChange = (command.pMaterial != pCurrMaterial);
        bool meshChange = ((pCurrIB != command.pIB) || (pCurrVB != command.pVB) ||
                           (currStartIndex != command.startIndex));

        //material has changed
        if (materialChange)
        {
            pCurrMaterial = command.pMaterial;
            SetMaterial(pCtx, pCurrMaterial);
        }

        //mesh has changed
        if (meshChange)
        {
            pCurrIB = command.pIB;
            pCurrVB = command.pVB;
            currStartIndex = command.startIndex;

            UINT stride = sizeof(MeshVertex);
            UINT offset = 0;
            RendererBufferD3D11* pIB = (RendererBufferD3D11*)pCurrIB;
            RendererBufferD3D11* pVB = (RendererBufferD3D11*)pCurrVB;
            pCtx->D3DContext->IASetVertexBuffers(0, 1, &pVB->D3DBuffer, &stride, &offset);
            pCtx->D3DContext->IASetIndexBuffer(pIB->D3DBuffer, DXGI_FORMAT_R32_UINT, 0);
        }

        //set instance matrix
        pCtx->D3DContext->UpdateSubresource(mPerInstanceCBuffer, 0, 0, &command.matrix, 0, 0);
        pCtx->D3DContext->DrawIndexed(command.indexCount, command.startIndex, 0);
    }
#endif
}

} // namespace Render
} // namespace NFE
