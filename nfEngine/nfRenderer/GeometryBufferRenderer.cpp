#include "PCH.hpp"
#include "GeometryBufferRenderer.hpp"
#include "Renderer.hpp"

namespace NFE {
namespace Render {

using namespace Math;

// include code shared with shaders
#include "Shaders/GBuffer.h"

struct MaterialCBuffer
{
    Vector diffuseColor;
    Vector specularColor;
    Vector emissionColor;
};

struct PerInstanceCBufferVS
{
    Matrix worldMatrix;
    Vector velocity;
    Vector angularVelocity;
};


#define INSTANCING (1)
#define MAX_BUFFERED_INSTANCES (8192)


GBufferRendererD3D11::GBufferRendererD3D11()
{
    mDepthStencilState = 0;
    mRasterizerState = 0;
    mInputLayout = 0;
    mSampler = 0;

    mInstancesVB = 0;
    mGlobalCBuffer = 0;
    mPerInstanceCBuffer = 0;

    mMaterialCBuffer = 0;
    mInstanceData = 0;
}

GBufferRendererD3D11::~GBufferRendererD3D11()
{

}

Result GBufferRendererD3D11::Init(RendererD3D11* pRenderer)
{
    HRESULT HR = 0;

    //compile shaders
    mShaderVS.AddMacro("INSTANCING", 1, 0);
    mShaderVS.AddMacro("USE_MOTION_BLUR", 1, 1);
    mShaderVS.Init(pRenderer, ShaderType::Vertex, "GeometryPassVS");

    mShaderPS.AddMacro("GAMMA_CORRECTION", 1, 0);
    mShaderPS.AddMacro("USE_MOTION_BLUR", 1, 1);
    mShaderPS.Init(pRenderer, ShaderType::Pixel, "GeometryPassPS");

    mShaderGS.Init(pRenderer, ShaderType::Geometry, "GeometryPassGS");



    D3D11_BUFFER_DESC bd;
    bd.MiscFlags = 0;

    //init Constant Buffers
    bd.ByteWidth = sizeof(XGeometryPassGlobalCBuffer);
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

    bd.ByteWidth = sizeof(MaterialCBuffer);
    HR |= D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mMaterialCBuffer));

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
        { "TEXCOORD",   4, DXGI_FORMAT_R32G32B32A32_FLOAT,  1,  48, D3D11_INPUT_PER_INSTANCE_DATA,  1},
        { "TEXCOORD",   5, DXGI_FORMAT_R32G32B32A32_FLOAT,  1,  64, D3D11_INPUT_PER_INSTANCE_DATA,  1},
    };

    //init Input Layout
    //UINT Instancing = INSTANCING;
    UINT macros[2] = {INSTANCING, 1};
    Common::Buffer* pShaderSource = mShaderVS.GetShaderBytecode(macros);

#if (INSTANCING > 0)
    D3D_CHECK(pRenderer->D3DDevice->CreateInputLayout(layout, 9, pShaderSource->GetData(),
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
    RendererD3D11DepthStencilDesc depthStencilDesc(true, D3D11_DEPTH_WRITE_MASK_ALL,
            D3D11_COMPARISON_LESS);
    D3D_CHECK(pRenderer->D3DDevice->CreateDepthStencilState(&depthStencilDesc,
              &mDepthStencilState));


    mInstanceData = (InstanceData*)_aligned_malloc(MAX_BUFFERED_INSTANCES * sizeof(InstanceData), 64);

    return Result::OK;
}

void GBufferRendererD3D11::Release()
{
    D3D_SAFE_RELEASE(mDepthStencilState);
    D3D_SAFE_RELEASE(mRasterizerState);
    D3D_SAFE_RELEASE(mInputLayout);

    D3D_SAFE_RELEASE(mMaterialCBuffer);
    D3D_SAFE_RELEASE(mGlobalCBuffer);
    D3D_SAFE_RELEASE(mPerInstanceCBuffer);

    D3D_SAFE_RELEASE(mSampler);
    D3D_SAFE_RELEASE(mInstancesVB);

    _aligned_free(mInstanceData);

    mShaderGS.Release();
    mShaderVS.Release();
    mShaderPS.Release();
}


void GBufferRendererD3D11::Enter(NFE_CONTEXT_ARG)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    auto pRenderer = pCtx->GetRenderer();
    GeometryBuffer& GeomBuffer = pRenderer->geomBuffer;

    //Clear G-buffer
    //TODO: motion buffer must be cleared with special shader
    if (pRenderer->settings.motionBlur)
    {
        //clear motion vectors
        float clearColor3[] = {0.0f, 0.0f, 0.0f, 0.0f};
        pCtx->D3DContext->ClearRenderTargetView(GeomBuffer.RTVs[3], clearColor3);
    }

    // bind G-buffer
    pCtx->D3DContext->ClearDepthStencilView(pRenderer->depthStencilView, D3D11_CLEAR_DEPTH, 1.0f,
                                            0);
    pCtx->D3DContext->OMSetRenderTargets(4, GeomBuffer.RTVs, pRenderer->depthStencilView);


    pCtx->D3DContext->IASetInputLayout(mInputLayout);
    pCtx->D3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D11Buffer* pCBuffersVS[] = {mGlobalCBuffer, mPerInstanceCBuffer};
    ID3D11Buffer* pCBuffersPS[] = {mGlobalCBuffer, mMaterialCBuffer};
    pCtx->D3DContext->VSSetConstantBuffers(0, 2, pCBuffersVS);
    pCtx->D3DContext->PSSetConstantBuffers(0, 2, pCBuffersPS);

    pCtx->D3DContext->OMSetDepthStencilState(mDepthStencilState, 0);
    pCtx->D3DContext->RSSetState(mRasterizerState);
    pCtx->D3DContext->PSSetSamplers(0, 1, &mSampler);

    //initialize per instance cbuffer with identity transformation
    PerInstanceCBufferVS CInstanceBufferData;
    CInstanceBufferData.worldMatrix = Matrix();
    CInstanceBufferData.velocity = Vector();
    CInstanceBufferData.angularVelocity = Vector();
    pCtx->D3DContext->UpdateSubresource(mPerInstanceCBuffer, 0, 0, &CInstanceBufferData, 0, 0);


    UINT macros[2] = {INSTANCING, pRenderer->settings.motionBlur};
    pCtx->BindShader(&mShaderVS, macros);
}

void GBufferRendererD3D11::Leave(NFE_CONTEXT_ARG)
{
    auto pCtx = (RenderContextD3D11*)pContext;

#if (INSTANCING == 1)
    //unbind instance vertex buffers
    ID3D11Buffer* pNullBuffer = 0;
    UINT stride = 0;
    UINT offset = 0;
    pCtx->D3DContext->IASetVertexBuffers(1, 1, &pNullBuffer, &stride, &offset);
#endif

    //unbind G-buffer from render target
    ID3D11RenderTargetView* pRTVs[] = {0, 0, 0, 0};
    pCtx->D3DContext->OMSetRenderTargets(4, pRTVs, 0);

    ID3D11ShaderResourceView* pNullSRVs[] = {0, 0, 0, 0, 0, 0};
    pCtx->D3DContext->PSSetShaderResources(0, 6, pNullSRVs);
}

void GBufferRendererD3D11::SetCamera(NFE_CONTEXT_ARG, const CameraRenderDesc* pCamera)
{
    auto pCtx = (RenderContextD3D11*)pContext;

    XGeometryPassGlobalCBuffer cbuffer;
    cbuffer.ProjMatrix = pCamera->projMatrix;
    cbuffer.ViewMatrix = pCamera->viewMatrix;
    cbuffer.SecondaryViewProjMatrix = pCamera->secViewMatrix;
    cbuffer.ViewProjMatrix = pCamera->viewMatrix * pCamera->projMatrix;
    cbuffer.CameraVelocity = pCamera->velocity;
    cbuffer.CameraAngularVelocity = pCamera->angualrVelocity;
    cbuffer.DeltaTime = 0.01f;
    pCtx->D3DContext->UpdateSubresource(mGlobalCBuffer, 0, 0, &cbuffer, 0, 0);
}

void GBufferRendererD3D11::SetTarget(NFE_CONTEXT_ARG, IRenderTarget* pTarget)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    RenderTargetD3D11* pRT = dynamic_cast<RenderTargetD3D11*>(pTarget);

    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (float)pRT->width;
    viewport.Height = (float)pRT->height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    pCtx->D3DContext->RSSetViewports(1, &viewport);
}

void GBufferRendererD3D11::SetMaterial(NFE_CONTEXT_ARG, const RendererMaterial* pMaterial)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    auto pRenderer = pCtx->GetRenderer();

    RendererTextureD3D11* pDiffuseTexture = 0;
    RendererTextureD3D11* pNormalTexture = 0;
    RendererTextureD3D11* pSpecularTexture = 0;

    MaterialCBuffer cbuffer;
    cbuffer.diffuseColor = Vector(1.0f, 1.0f, 1.0f, 1.0f);
    cbuffer.specularColor = Vector(1.0f, 1.0f, 1.0f, 1.0f);
    cbuffer.emissionColor = Vector();

    // TODO: textures layering
    if (pMaterial)
    {
        RendererMaterialLayer* pLayer = &pMaterial->layers[0];

        if (pLayer)
        {
            pDiffuseTexture     = (pLayer->diffuseTex != NULL) ? dynamic_cast<RendererTextureD3D11*>
                                  (pLayer->diffuseTex) : NULL;
            pNormalTexture      = (pLayer->normalTex != NULL) ? dynamic_cast<RendererTextureD3D11*>
                                  (pLayer->normalTex) : NULL;
            pSpecularTexture    = (pLayer->specularTex != NULL) ? dynamic_cast<RendererTextureD3D11*>
                                  (pLayer->specularTex) : NULL;

            cbuffer.diffuseColor = pMaterial->layers[0].diffuseColor;
            cbuffer.specularColor = pMaterial->layers[0].specularColor;
            cbuffer.emissionColor = pMaterial->layers[0].emissionColor;
        }
    }


    pCtx->D3DContext->UpdateSubresource(mMaterialCBuffer, 0, 0, &cbuffer, 0, 0);

    UINT macros[2];
    macros[0] = pRenderer->settings.gammaCorrection ? 1 : 0;
    macros[1] = pRenderer->settings.motionBlur ? 1 : 0;
    pCtx->BindShader(&mShaderPS, macros);

    if (pDiffuseTexture == 0)
        pDiffuseTexture = pRenderer->defaultDiffuseTexture;

    if (pNormalTexture == 0)
        pNormalTexture = pRenderer->defaultNormalTexture;

    if (pSpecularTexture == 0)
        pSpecularTexture = pRenderer->defaultSpecularTexture;

    //set shader resource views
    ID3D11ShaderResourceView* pSRVs[3];
    pSRVs[0] = pDiffuseTexture->SRV;
    pSRVs[1] = pNormalTexture->SRV;
    pSRVs[2] = pSpecularTexture->SRV;
    pCtx->D3DContext->PSSetShaderResources(0, 3, pSRVs);
}


void GBufferRendererD3D11::Draw(NFE_CONTEXT_ARG, const RenderCommandBuffer& buffer)
{
    auto pCtx = (RenderContextD3D11*)pContext;

#if (INSTANCING > 0)

    if (buffer.commands.size() <= 0)
        return;

    //bind instances VB
    UINT strides[] = {sizeof(InstanceData)};
    UINT offsets[] = {0};
    pCtx->D3DContext->IASetVertexBuffers(1, 1, &mInstancesVB, strides, offsets);


    //Material* pCurrMaterial = 0;
    IRendererBuffer* pCurrVB = NULL;
    IRendererBuffer* pCurrIB = NULL;
    uint32 currStartIndex = 0xFFFFFFFF;
    uint32 currIndexCount = 0;

    const RendererMaterial* currMaterial = (RendererMaterial*)(-1);
    int bufferedInstances = 0;
    uint32 startInstanceLocation = 0;

    for (size_t i = 0; i < buffer.commands.size(); i++)
    {
        const RenderCommand& command = buffer.commands[i];

        bool bufferIsFull = (bufferedInstances + bufferedInstances >= MAX_BUFFERED_INSTANCES) || (i == 0);
        bool materialChange = (command.pMaterial != currMaterial);
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
            size_t instancesToBuffer = Min<size_t>(MAX_BUFFERED_INSTANCES, buffer.commands.size() - i);

            D3D11_MAPPED_SUBRESOURCE mappedSubres;
            for (size_t j = 0; j < instancesToBuffer; j++)
            {
                Matrix tmpMatrix = MatrixTranspose(buffer.commands[j + i].matrix);
                mInstanceData[j].worldMatrix[0] = tmpMatrix.r[0];
                mInstanceData[j].worldMatrix[1] = tmpMatrix.r[1];
                mInstanceData[j].worldMatrix[2] = tmpMatrix.r[2];
                mInstanceData[j].velocity = buffer.commands[j + i].velocity;
                mInstanceData[j].angularVelocity = buffer.commands[j + i].angularVelocity;
            }
            pCtx->D3DContext->Map(mInstancesVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubres);
            memcpy(mappedSubres.pData, mInstanceData, instancesToBuffer * sizeof(InstanceData));
            pCtx->D3DContext->Unmap(mInstancesVB, 0);
            bufferedInstances = startInstanceLocation = 0;
        }

        //material has changed
        if (materialChange)
        {
            currMaterial = command.pMaterial;
            SetMaterial(pCtx, currMaterial);
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
    Mesh* pCurrMesh = 0;
    uint32 currStartIndex = 0xFFFFFFFF;

    for (int i = 0; i < buffer.m_Commands.size(); i++)
    {
        const RenderCommand& command = buffer.m_Commands[i];

        bool materialChange = (command.pMaterial != currMaterial);
        bool meshChange = ((pCurrMesh != command.pMesh) || (currStartIndex != command.startIndex));

        //material has changed
        if (materialChange)
        {
            currMaterial = command.pMaterial;
            SetMaterial(currMaterial);
        }

        //mesh has changed
        if (meshChange)
        {
            pCurrMesh = command.pMesh;
            currStartIndex = command.startIndex;

            UINT stride = sizeof(MeshVertex);
            UINT offset = 0;
            pCtx->D3DContext->IASetVertexBuffers(0, 1, &pCurrMesh->m_pVB->D3DBuffer, &stride, &offset);
            pCtx->D3DContext->IASetIndexBuffer(pCurrMesh->m_pIB->D3DBuffer, DXGI_FORMAT_R32_UINT, 0);
        }

        //set instance matrix
        pCtx->D3DContext->UpdateSubresource(mPerInstanceCBuffer, 0, 0, &command.matrix, 0, 0);
        pCtx->D3DContext->DrawIndexed(command.indexCount, command.startIndex, 0);
    }
#endif

}

} // namespace Render
} // namespace NFE
