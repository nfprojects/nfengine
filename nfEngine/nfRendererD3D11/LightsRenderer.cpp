#include "stdafx.hpp"
#include "LightsRenderer.hpp"
#include "Renderer.hpp"

namespace NFE {
namespace Render {

using namespace Math;

#define MAX_LIGHTS_TILE_BASED (1024)
#define MAX_TILES_FRUSTUMS (512)

struct LightTileGlobalCBuffer
{
    uint32 outResolution[4];
    Matrix ViewProjInverseMatrix;
    Matrix ProjInverseMatrix;
    Matrix ViewInverseMatrix;
    Matrix ProjMatrix;
    Matrix ViewMatrix;
    Matrix CameraMatrix;
    Vector ScreenScale;

    Vector ambientLight;
    Vector backgroundColor;
};

struct LightTilePerPassCBuffer
{
    uint32 LightsCount[4];
};

LightsRendererD3D11::LightsRendererD3D11()
{
    mTarget = 0;

    mAdditiveBlendState = 0;
    mFogBlendState = 0;
    mDepthStencilState = 0;
    mRasterizerState = 0;
    mInputLayout = 0;

    mShadowSampler = 0;
    mLightMapSampler = 0;
    mPointSampler = 0;

    mGlobalCBuffer = 0;
    mOmniLightCBuffer = 0;
    mSpotLightCBuffer = 0;
    mAmbientLightCBuffer = 0;
    mDirLightCBuffer = 0;

    mLightTileGlobalCBuffer = 0;
    mLightTilePerPassCBuffer = 0;
    mLightsBuffer = 0;
    mLightsBufferSRV = 0;
    mTilesFrustumsBuffer = 0;
    mTilesFrustumsBufferSRV = 0;


    mIcosahedronVB = 0;
    mLightIB = 0;
    mSpotLightVB = 0;
}

LightsRendererD3D11::~LightsRendererD3D11()
{
    Release();
}

Result LightsRendererD3D11::Init(RendererD3D11* pRenderer)
{
    HRESULT HR = 0;

    //compile shaders
    mOmniLightShaderVS.Init(pRenderer, ShaderType::Vertex, "OmniLightVS");
    mOmniLightShaderPS.AddMacro("USE_SHADOWS", 1, 0);
    mOmniLightShaderPS.Init(pRenderer, ShaderType::Pixel, "OmniLightPS");


    mSpotLightShaderVS.Init(pRenderer, ShaderType::Vertex, "SpotLightVS");
    mSpotLightShaderPS.AddMacro("USE_SHADOWS", 1, 0);
    mSpotLightShaderPS.AddMacro("USE_LIGHT_MAP", 1, 1);
    mSpotLightShaderPS.Init(pRenderer, ShaderType::Pixel, "SpotLightPS");

    mDirLightCS.AddMacro("USE_SHADOWS", 1, 0);
    mDirLightCS.Init(pRenderer, ShaderType::Pixel, "DirLightPS");


    mGlobalFogPS.Init(pRenderer, ShaderType::Pixel, "GlobalFogPS");
    mAmbientLightPS.Init(pRenderer, ShaderType::Pixel, "AmbientLightPS");
    if (pRenderer->featureLevel == D3D_FEATURE_LEVEL_11_0)
    {
        mAmbientLightCS.Init(pRenderer, ShaderType::Compute, "AmbientLightCS");
        mLightTileCS.Init(pRenderer, ShaderType::Compute, "LightTileCS");
    }



    D3D11_SUBRESOURCE_DATA initData;
    D3D11_BUFFER_DESC bd;
    bd.MiscFlags = 0;

    //init Constant Buffers
    bd.ByteWidth = sizeof(LightGlobalCBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.StructureByteStride = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    HR |= D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mGlobalCBuffer));

    bd.ByteWidth = sizeof(OmniLightProperties);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.StructureByteStride = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    HR |= D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mOmniLightCBuffer));

    bd.ByteWidth = sizeof(SpotLightProperties);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.StructureByteStride = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    HR |= D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mSpotLightCBuffer));

    bd.ByteWidth = sizeof(AmbientLightCBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.StructureByteStride = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    HR |= D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mAmbientLightCBuffer));

    bd.ByteWidth = sizeof(DirLightProperties);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.StructureByteStride = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    HR |= D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mDirLightCBuffer));


    // TILE-BASED DEFERRED SHADING
    bd.ByteWidth = sizeof(LightTileGlobalCBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.StructureByteStride = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    HR |= D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mLightTileGlobalCBuffer));

    bd.ByteWidth = sizeof(LightTilePerPassCBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.StructureByteStride = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    HR |= D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mLightTilePerPassCBuffer));

    bd.ByteWidth = MAX_LIGHTS_TILE_BASED * sizeof(TileOmniLightDesc);
    bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.StructureByteStride = sizeof(TileOmniLightDesc);
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    HR |= D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mLightsBuffer));
    HR |= D3D_CHECK(pRenderer->D3DDevice->CreateShaderResourceView(mLightsBuffer, 0,
                    &mLightsBufferSRV));



    bd.ByteWidth = MAX_TILES_FRUSTUMS * sizeof(Vector);
    bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.StructureByteStride = sizeof(Vector);
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    HR |= D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mTilesFrustumsBuffer));
    HR |= D3D_CHECK(pRenderer->D3DDevice->CreateShaderResourceView(mTilesFrustumsBuffer, 0,
                    &mTilesFrustumsBufferSRV));



    //create input layout
    const D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    //init Input Layout
    Common::Buffer* pShaderSource = mOmniLightShaderVS.GetShaderBytecode(0);
    D3D_CHECK(pRenderer->D3DDevice->CreateInputLayout(layout, 1, pShaderSource->GetData(),
              pShaderSource->GetSize(), &mInputLayout));


    // Initialize depth stencil state for rendering light volumes (front face culling)
    D3D11_RASTERIZER_DESC rd;
    ZeroMemory(&rd, sizeof(rd));
    rd.CullMode = D3D11_CULL_FRONT;
    rd.FrontCounterClockwise = 1;
    rd.FillMode = D3D11_FILL_SOLID;
    rd.DepthClipEnable = 0;
    D3D_CHECK(pRenderer->D3DDevice->CreateRasterizerState(&rd, &mRasterizerState));


    // Initialize depth stencil state
    RendererD3D11DepthStencilDesc depthStencilDesc(1, D3D11_DEPTH_WRITE_MASK_ZERO,
            D3D11_COMPARISON_GREATER);
    D3D_CHECK(pRenderer->D3DDevice->CreateDepthStencilState(&depthStencilDesc,
              &mDepthStencilState));


    //addidtive blendig
    D3D11_BLEND_DESC blendDesc;
    blendDesc.AlphaToCoverageEnable = 0;
    blendDesc.IndependentBlendEnable = 0;
    blendDesc.RenderTarget[0].BlendEnable = 1;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    D3D_CHECK(pRenderer->D3DDevice->CreateBlendState(&blendDesc, &mAdditiveBlendState));

    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_ALPHA;
    D3D_CHECK(pRenderer->D3DDevice->CreateBlendState(&blendDesc, &mFogBlendState));


    // Icosahedron verticies (for omni lights)
    float scale = 0.6616f;
    float phi = 1.6180345f * scale;
    Float3 pVerticies[] =
    {
        Float3( 0.0f,  -phi,    -scale),
        Float3( 0.0f,  phi, -scale),
        Float3( 0.0f,  phi, scale),
        Float3( 0.0f,  -phi,    scale),

        Float3( -scale, 0.0f, -phi),
        Float3( -scale, 0.0f, phi),
        Float3( scale,  0.0f, phi),
        Float3( scale,  0.0f, -phi),

        Float3( -phi,   -scale, 0.0f),
        Float3( -phi,  scale,   0.0f),
        Float3( phi,    scale,  0.0f),
        Float3( phi,    -scale, 0.0f),

    };

    bd.MiscFlags = 0;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.ByteWidth = 12 * sizeof(Float3);
    bd.CPUAccessFlags = 0;
    bd.Usage = D3D11_USAGE_IMMUTABLE;
    bd.StructureByteStride = sizeof(Float3);
    initData.pSysMem = pVerticies;
    initData.SysMemPitch = 0;
    initData.SysMemSlicePitch = 0;
    HR |= D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, &initData, &mIcosahedronVB));


    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.ByteWidth = 8 * sizeof(Float3);
    bd.CPUAccessFlags = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.StructureByteStride = sizeof(Float3);
    HR |= D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mSpotLightVB));

    uint16 pIndices[] =
    {
        //indices for box / cube
        0, 1, 3, 0, 3, 2, // Z- face
        7, 5, 4, 7, 4, 6, // Z+ face
        6, 4, 0, 6, 0, 2, // X- face
        7, 3, 1, 7, 1, 5, // X+ face
        7, 2, 3, 7, 6, 2, // Y+ face
        5, 1, 0, 5, 0, 4,  // Y- face

        //indices for
        2, 1, 10, 2, 10, 6, 2, 6, 5, 2, 5, 9, 2, 9, 1, //top cap
        0, 11, 7, 0, 7, 4, 0, 4, 8, 0, 8, 3, 0, 3, 11, //bottom cap
        9, 4, 1, 4, 7, 1, 7, 10, 1, 7, 11, 10, 11, 6, 10, 6, 11, 3, 6, 3, 5, 5, 3, 8, 9, 5, 8, 9, 8, 4, //rest
    };

    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.ByteWidth = (6 * 6 + 3 * 20) * sizeof(uint16);
    bd.CPUAccessFlags = 0;
    bd.Usage = D3D11_USAGE_IMMUTABLE;
    bd.StructureByteStride = sizeof(uint16);
    initData.pSysMem = pIndices;
    HR |= D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, &initData, &mLightIB));


    RendererD3D11SamplerDesc samplerDesc(D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
                                         D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_LESS_EQUAL, 1);
    D3D_CHECK(pRenderer->D3DDevice->CreateSamplerState(&samplerDesc, &mShadowSampler));


    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MaxAnisotropy = 16;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    samplerDesc.MinLOD = -D3D11_FLOAT32_MAX;
    D3D_CHECK(pRenderer->D3DDevice->CreateSamplerState(&samplerDesc, &mLightMapSampler));


    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.MaxLOD = 0;
    samplerDesc.MinLOD = 0;
    D3D_CHECK(pRenderer->D3DDevice->CreateSamplerState(&samplerDesc, &mPointSampler));

    return Result::OK;
}

void LightsRendererD3D11::Release()
{
    D3D_SAFE_RELEASE(mAdditiveBlendState);
    D3D_SAFE_RELEASE(mFogBlendState);
    D3D_SAFE_RELEASE(mDepthStencilState);
    D3D_SAFE_RELEASE(mRasterizerState);
    D3D_SAFE_RELEASE(mInputLayout);

    D3D_SAFE_RELEASE(mShadowSampler);
    D3D_SAFE_RELEASE(mLightMapSampler);
    D3D_SAFE_RELEASE(mPointSampler);

    D3D_SAFE_RELEASE(mGlobalCBuffer);
    D3D_SAFE_RELEASE(mOmniLightCBuffer);
    D3D_SAFE_RELEASE(mSpotLightCBuffer);
    D3D_SAFE_RELEASE(mAmbientLightCBuffer);
    D3D_SAFE_RELEASE(mDirLightCBuffer);
    D3D_SAFE_RELEASE(mLightTileGlobalCBuffer);
    D3D_SAFE_RELEASE(mLightTilePerPassCBuffer);

    D3D_SAFE_RELEASE(mLightsBufferSRV);
    D3D_SAFE_RELEASE(mLightsBuffer);

    D3D_SAFE_RELEASE(mTilesFrustumsBufferSRV);
    D3D_SAFE_RELEASE(mTilesFrustumsBuffer);

    D3D_SAFE_RELEASE(mIcosahedronVB);
    D3D_SAFE_RELEASE(mLightIB);
    D3D_SAFE_RELEASE(mSpotLightVB);

    //release shaders
    mGlobalFogPS.Release();
    mOmniLightShaderVS.Release();
    mOmniLightShaderPS.Release();
    mSpotLightShaderVS.Release();
    mSpotLightShaderPS.Release();
    mLightTileCS.Release();
    mDirLightCS.Release();
    mAmbientLightPS.Release();
    mAmbientLightCS.Release();
}

void LightsRendererD3D11::Enter(NFE_CONTEXT_ARG)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    auto pRenderer = pCtx->GetRenderer();

    //bind g-buffer as input
    pCtx->D3DContext->PSSetShaderResources(0, 4, pRenderer->geomBuffer.SRVs);
    pCtx->D3DContext->PSSetShaderResources(4, 1, &pRenderer->depthStencilSRV);
    ID3D11SamplerState* pSamplers[] = {mShadowSampler, mLightMapSampler, mPointSampler};
    pCtx->D3DContext->PSSetSamplers(0, 3, pSamplers);
}

void LightsRendererD3D11::SetUp(NFE_CONTEXT_ARG, IRenderTarget* pRT,
                                const CameraRenderDesc* pCamera, const Vector& ambientLightColor, const Vector& backgroundColor)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    auto pRenderer = pCtx->GetRenderer();
    mTarget = dynamic_cast<RenderTargetD3D11*>(pRT);

    Matrix viewProjMatrix = pCamera->viewMatrix * pCamera->projMatrix;
    Matrix viewProjInverse = MatrixInverse(viewProjMatrix);

    //update global c-buffer
    LightGlobalCBuffer CBufferData;
    CBufferData.cameraMatrix = MatrixTranspose(pCamera->matrix);
    CBufferData.projMatrix = pCamera->projMatrix;
    CBufferData.viewMatrix = pCamera->viewMatrix;
    CBufferData.projInverseMatrix = MatrixTranspose(MatrixInverse(pCamera->projMatrix));
    CBufferData.viewProjMatrix = viewProjMatrix;//pCamera->mViewMatrix * pCamera->mProjMatrix;
    CBufferData.viewProjInverseMatrix = MatrixTranspose(viewProjInverse);
    CBufferData.viewportResInv = Vector(1.0f / mTarget->width, 1.0f / mTarget->height, 0.0f,
                                        0.0f);
    CBufferData.screenScale = pCamera->screenScale;
    pCtx->D3DContext->UpdateSubresource(mGlobalCBuffer, 0, 0, &CBufferData, 0, 0);


    //update global c-buffer for tile based rendering
    LightTileGlobalCBuffer lightTileGlobalCBuffer;
    lightTileGlobalCBuffer.outResolution[0] = mTarget->width;
    lightTileGlobalCBuffer.outResolution[1] = mTarget->height;
    lightTileGlobalCBuffer.ViewProjInverseMatrix = CBufferData.viewProjInverseMatrix;
    lightTileGlobalCBuffer.ViewInverseMatrix = MatrixTranspose(MatrixInverse(pCamera->viewMatrix));
    lightTileGlobalCBuffer.ProjInverseMatrix = MatrixTranspose(MatrixInverse(pCamera->projMatrix));
    lightTileGlobalCBuffer.ProjMatrix = MatrixTranspose(pCamera->projMatrix);
    lightTileGlobalCBuffer.ViewMatrix = pCamera->viewMatrix;
    lightTileGlobalCBuffer.CameraMatrix = MatrixTranspose(pCamera->matrix);
    lightTileGlobalCBuffer.ScreenScale = pCamera->screenScale;
    lightTileGlobalCBuffer.ambientLight = ambientLightColor;
    lightTileGlobalCBuffer.backgroundColor = backgroundColor;
    pCtx->D3DContext->UpdateSubresource(mLightTileGlobalCBuffer, 0, 0, &lightTileGlobalCBuffer, 0,
                                        0);


    //update global c-buffer ambient & fog rendering pass
    AmbientLightCBuffer ambientLight;
    //ambient light parameters
    VectorStore(ambientLightColor, &ambientLight.ambientLight);
    VectorStore(backgroundColor, &ambientLight.backgroundColor);
    //output resolution
    ambientLight.outResolution.x = (float)mTarget->width;
    ambientLight.outResolution.y = (float)mTarget->height;
    ambientLight.outResolution.z = 1.0f / (float)mTarget->width;
    ambientLight.outResolution.w = 1.0f / (float)mTarget->height;
    //for HBAO
    ambientLight.focalLen.x = 1.0f / tanf(pCamera->fov * 0.5f) *  (float)mTarget->width /
                              (float)mTarget->height;
    ambientLight.focalLen.y = 1.0f / tanf(pCamera->fov * 0.5f);
    ambientLight.focalLen.z = 1.0f / ambientLight.focalLen.x;
    ambientLight.focalLen.w = 1.0f / ambientLight.focalLen.y;
    pCtx->D3DContext->UpdateSubresource(mAmbientLightCBuffer, 0, 0, &ambientLight, 0, 0);


    if (pRenderer->settings.tileBasedDeferredShading)
    {
        // ------------------------------------------------------
        // CALCULATE TILES FRUSTUMS -----------------------------
        // ------------------------------------------------------
        Vector* pPlanes = (Vector*)_aligned_malloc(MAX_TILES_FRUSTUMS * sizeof(Vector), 16);
        ZeroMemory(pPlanes, MAX_TILES_FRUSTUMS * sizeof(Vector));

        Vector vertexA, vertexB, vertexC;
        vertexA = pCamera->matrix.r[3];

        uint32 planeId = 0;
        for (uint32 x = 0; x < mTarget->width; x += 16)
        {
            vertexB = Vector(2.0f * (float)x / (float)mTarget->width - 1.0f, 0.0f, 1.0f, 1.0f);
            vertexC = Vector(2.0f * (float)x / (float)mTarget->width - 1.0f, 1.0f, 1.0f, 1.0f);
            vertexB = VectorTransform4(vertexB, viewProjInverse);
            vertexC = VectorTransform4(vertexC, viewProjInverse);
            vertexB /= vertexB.f[3];
            vertexC /= vertexC.f[3];
            pPlanes[planeId++] = PlaneFromPoints(vertexA, vertexB, vertexC);
        }
        vertexB = Vector(1.0f, 0.0f, 1.0f, 1.0f);
        vertexC = Vector(1.0f, 1.0f, 1.0f, 1.0f);
        vertexB = VectorTransform4(vertexB, viewProjInverse);
        vertexC = VectorTransform4(vertexC, viewProjInverse);
        vertexB /= vertexB.f[3];
        vertexC /= vertexC.f[3];
        pPlanes[planeId] = PlaneFromPoints(vertexA, vertexB, vertexC);


        planeId = MAX_TILES_FRUSTUMS / 2;
        for (uint32 y = 0; y < mTarget->height; y += 16)
        {
            vertexB = Vector(0.0f, 1.0f - 2.0f * (float)y / (float)mTarget->height, 1.0f, 1.0f);
            vertexC = Vector(1.0f, 1.0f - 2.0f * (float)y / (float)mTarget->height, 1.0f, 1.0f);
            vertexB = VectorTransform4(vertexB, viewProjInverse);
            vertexC = VectorTransform4(vertexC, viewProjInverse);
            vertexB /= vertexB.f[3];
            vertexC /= vertexC.f[3];
            pPlanes[planeId++] = PlaneFromPoints(vertexA, vertexB, vertexC);
        }
        vertexB = Vector(0.0f, 0.0f, 1.0f, 1.0f);
        vertexC = Vector(1.0f, 0.0f, 1.0f, 1.0f);
        vertexB = VectorTransform4(vertexB, viewProjInverse);
        vertexC = VectorTransform4(vertexC, viewProjInverse);
        vertexB /= vertexB.f[3];
        vertexC /= vertexC.f[3];
        pPlanes[planeId] = PlaneFromPoints(vertexA, vertexB, vertexC);


        //copy lights to GPU
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        pCtx->D3DContext->Map(mTilesFrustumsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        memcpy(mappedResource.pData, pPlanes, MAX_TILES_FRUSTUMS * sizeof(Vector));
        pCtx->D3DContext->Unmap(mTilesFrustumsBuffer, 0);

        _aligned_free(pPlanes);
    }


    //clear render target
    //float clearColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
    //pCtx->D3DContext->ClearRenderTargetView(pTarget->RTV, clearColor);

    //set up Input Assembler
    pCtx->D3DContext->IASetInputLayout(mInputLayout);
    pCtx->D3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    if (!pRenderer->settings.tileBasedDeferredShading)
    {
        //pCtx->D3DContext->ClearRenderTargetView(pRenderer->mRenderTarget.RTV, backgroundColor.f);
        pCtx->D3DContext->OMSetRenderTargets(1, &(pRenderer->renderTarget.RTV), 0);
        DrawAmbientLight(pCtx, ambientLightColor, backgroundColor);
    }

    pCtx->D3DContext->OMSetBlendState(mAdditiveBlendState, 0, 0xFFFFFFFF);
    pCtx->D3DContext->OMSetDepthStencilState(mDepthStencilState, 0);
    pCtx->D3DContext->RSSetState(mRasterizerState);

    ID3D11Buffer* pCBuffers[] = {mGlobalCBuffer};
    pCtx->D3DContext->VSSetConstantBuffers(0, 1, pCBuffers);
    pCtx->D3DContext->PSSetConstantBuffers(0, 1, pCBuffers);
}

void LightsRendererD3D11::Leave(NFE_CONTEXT_ARG)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    auto pRenderer = pCtx->GetRenderer();

    //unbound g-buffer
    ID3D11ShaderResourceView* pNullSRVs[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    pCtx->D3DContext->PSSetShaderResources(0, 10, pNullSRVs);
    pCtx->D3DContext->CSSetShaderResources(0, 10, pNullSRVs);
    pCtx->D3DContext->OMSetBlendState(pRenderer->defaultBlendState, 0, 0xFFFFFFFF);

    ID3D11RenderTargetView* pNullRTVs[] = {0};
    pCtx->D3DContext->OMSetRenderTargets(1, pNullRTVs, 0);
}


void LightsRendererD3D11::TileBasedPass(NFE_CONTEXT_ARG, uint32 lightsCount,
                                        const TileOmniLightDesc* pLights)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    auto pRenderer = pCtx->GetRenderer();

    if (lightsCount > MAX_LIGHTS_TILE_BASED)
        lightsCount = MAX_LIGHTS_TILE_BASED;

    LightTilePerPassCBuffer CBufferData;
    CBufferData.LightsCount[0] = lightsCount;
    pCtx->D3DContext->UpdateSubresource(mLightTilePerPassCBuffer, 0, 0, &CBufferData, 0, 0);

    //copy lights to GPU
    if (lightsCount)
    {
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        pCtx->D3DContext->Map(mLightsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        memcpy(mappedResource.pData, pLights, lightsCount * sizeof(TileOmniLightDesc));
        pCtx->D3DContext->Unmap(mLightsBuffer, 0);
    }

    ID3D11UnorderedAccessView* pNullUAV = 0;
    ID3D11Buffer* pCBuffers[] = {mLightTileGlobalCBuffer, mLightTilePerPassCBuffer};

    pCtx->BindShader(&mLightTileCS, 0);
    pCtx->D3DContext->CSSetConstantBuffers(0, 2, pCBuffers);
    pCtx->D3DContext->CSSetShaderResources(0, 4, pRenderer->geomBuffer.SRVs);
    pCtx->D3DContext->CSSetShaderResources(4, 1, &pRenderer->depthStencilSRV);
    pCtx->D3DContext->CSSetShaderResources(5, 1, &mLightsBufferSRV); //bind lights buffer
    pCtx->D3DContext->CSSetShaderResources(6, 1,
                                           &mTilesFrustumsBufferSRV); //bind precomputed tiles frustums

    pCtx->D3DContext->CSSetUnorderedAccessViews(0, 1, &pRenderer->renderTarget.UAV, 0);
    pCtx->D3DContext->Dispatch(mTarget->width / 16 + 1, mTarget->height / 16 + 1, 1);
    pCtx->D3DContext->CSSetUnorderedAccessViews(0, 1, &pNullUAV, 0);

    //switch to additive rendering
    pCtx->D3DContext->OMSetRenderTargets(1, &(pRenderer->renderTarget.RTV), 0);
}

void LightsRendererD3D11::DrawAmbientLight(NFE_CONTEXT_ARG, const Vector& ambientLightColor,
        const Vector& backgroundColor)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    auto pRenderer = pCtx->GetRenderer();

    //bind constant buffer with ambient light
    pCtx->D3DContext->PSSetConstantBuffers(1, 1, &mAmbientLightCBuffer);

    //bind IB and VB for quad rendering
    UINT strides[] = {sizeof(Float3)};
    UINT offsets[] = {0};
    pCtx->D3DContext->IASetVertexBuffers(0, 1, &pRenderer->screenQuadVB, strides, offsets);
    pCtx->D3DContext->IASetIndexBuffer(pRenderer->screenQuadIB, DXGI_FORMAT_R32_UINT, 0);

    pCtx->D3DContext->OMSetRenderTargets(1, &pRenderer->renderTarget.RTV,
                                         0); //pRenderer->depthStencilView
    pCtx->D3DContext->OMSetDepthStencilState(pRenderer->defaultDepthStencilState, 0);
    pCtx->D3DContext->OMSetBlendState(pRenderer->defaultBlendState, 0, 0xFFFFFFFF);
    pCtx->D3DContext->RSSetState(pRenderer->defaultRasterizerState);

    //bind shaders
    pCtx->BindShader(&pRenderer->fullScreenQuadVS, 0);
    pCtx->BindShader(&mAmbientLightPS, 0);

    pCtx->D3DContext->DrawIndexed(6, 0, 0);
}

void LightsRendererD3D11::DrawFog(NFE_CONTEXT_ARG)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    auto pRenderer = pCtx->GetRenderer();

    //bind constant buffer with ambient light
    pCtx->D3DContext->PSSetConstantBuffers(1, 1, &mAmbientLightCBuffer);

    //bind IB and VB for quad rendering
    UINT strides[] = {sizeof(Float3)};
    UINT offsets[] = {0};
    pCtx->D3DContext->IASetVertexBuffers(0, 1, &pRenderer->screenQuadVB, strides, offsets);
    pCtx->D3DContext->IASetIndexBuffer(pRenderer->screenQuadIB, DXGI_FORMAT_R32_UINT, 0);

    pCtx->D3DContext->OMSetRenderTargets(1, &pRenderer->renderTarget.RTV,
                                         0); //pRenderer->depthStencilView
    pCtx->D3DContext->OMSetDepthStencilState(pRenderer->defaultDepthStencilState, 0);
    pCtx->D3DContext->OMSetBlendState(mFogBlendState, 0, 0xFFFFFFFF);
    pCtx->D3DContext->RSSetState(pRenderer->defaultRasterizerState);

    //bind shaders
    pCtx->BindShader(&pRenderer->fullScreenQuadVS, 0);
    pCtx->BindShader(&mGlobalFogPS, 0);

    pCtx->D3DContext->DrawIndexed(6, 0, 0);
}

void LightsRendererD3D11::DrawOmniLight(NFE_CONTEXT_ARG, const Vector& pos, float radius,
                                        const Vector& color, IShadowMap* pShadowMap)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    ShadowMapD3D11* pSM = (ShadowMapD3D11*)pShadowMap;

    OmniLightProperties CBufferData;
    CBufferData.position = pos;
    CBufferData.radius = VectorSplat(radius);
    CBufferData.color = color;
    CBufferData.shadowMapResInv = (pSM != nullptr) ? Vector(1.0f / pSM->size) : Vector();
    pCtx->D3DContext->UpdateSubresource(mOmniLightCBuffer, 0, 0, &CBufferData, 0, 0);


    pCtx->D3DContext->VSSetConstantBuffers(1, 1, &mOmniLightCBuffer);
    pCtx->D3DContext->PSSetConstantBuffers(1, 1, &mOmniLightCBuffer);

    UINT strides[] = {sizeof(Float3)};
    UINT offsets[] = {0};
    pCtx->D3DContext->IASetVertexBuffers(0, 1, &mIcosahedronVB, strides, offsets);
    pCtx->D3DContext->IASetIndexBuffer(mLightIB, DXGI_FORMAT_R16_UINT, 0);

    uint32 useShadowMap = 0;
    if (pSM)
    {
        if (pSM->size)
        {
            //bind shadow map
            useShadowMap = 1;
            pCtx->D3DContext->PSSetShaderResources(8, 1, &(pSM->SRV));
        }
    }


    pCtx->BindShader(&mOmniLightShaderVS, 0);
    pCtx->BindShader(&mOmniLightShaderPS, &useShadowMap);

    pCtx->D3DContext->RSSetState(mRasterizerState);
    pCtx->D3DContext->DrawIndexed(20 * 3, 36, 0);
}


void LightsRendererD3D11::DrawSpotLight(NFE_CONTEXT_ARG, const SpotLightProperties& prop,
                                        const Frustum& frustum,
                                        IShadowMap* pShadowMap, IRendererTexture* pLightMap)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    ShadowMapD3D11* pSM = (ShadowMapD3D11*)pShadowMap;

    pCtx->D3DContext->UpdateSubresource(mSpotLightCBuffer, 0, 0, &prop, 0, 0);
    pCtx->D3DContext->VSSetConstantBuffers(1, 1, &mSpotLightCBuffer);
    pCtx->D3DContext->PSSetConstantBuffers(1, 1, &mSpotLightCBuffer);


    uint32 useTextures[2];
    useTextures[0] = 0;
    useTextures[1] = 0;

    if (pSM)
    {
        if (pSM->size)
        {
            //bind shadow map
            useTextures[0] = 1;
            pCtx->D3DContext->PSSetShaderResources(8, 1, &(pSM->SRV));
        }
    }

    RendererTextureD3D11* pTex = dynamic_cast<RendererTextureD3D11*>(pLightMap);
    if (pTex)
    {
        //bind shadow map
        useTextures[1] = 1;
        pCtx->D3DContext->PSSetShaderResources(9, 1, &pTex->SRV);
    }

    pCtx->BindShader(&mSpotLightShaderVS, 0);
    pCtx->BindShader(&mSpotLightShaderPS, useTextures);


    Float3 verticies[8];
    for (int i = 0; i < 8; i++)
        VectorStore(frustum.verticies[i], &verticies[i]);
    pCtx->D3DContext->UpdateSubresource(mSpotLightVB, 0, 0, verticies, 0, 0);

    UINT strides[] = {sizeof(Float3)};
    UINT offsets[] = {0};
    pCtx->D3DContext->IASetVertexBuffers(0, 1, &mSpotLightVB, strides, offsets);
    pCtx->D3DContext->IASetIndexBuffer(mLightIB, DXGI_FORMAT_R16_UINT, 0);

    pCtx->D3DContext->RSSetState(mRasterizerState);
    pCtx->D3DContext->DrawIndexed(36, 0, 0);
}

void LightsRendererD3D11::DrawDirLight(NFE_CONTEXT_ARG, const DirLightProperties& prop,
                                       IShadowMap* pShadowMap)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    auto pRenderer = pCtx->GetRenderer();
    ShadowMapD3D11* pSM = (ShadowMapD3D11*)pShadowMap;

    //bind constant buffer with dir light properties
    pCtx->D3DContext->PSSetConstantBuffers(1, 1, &mDirLightCBuffer);
    pCtx->D3DContext->UpdateSubresource(mDirLightCBuffer, 0, 0, &prop, 0, 0);

    if (pSM)
    {
        if (pSM->size)
        {
            //bind shadow map
            pCtx->D3DContext->PSSetShaderResources(8, 1, &(pSM->SRV));
        }
    }

    //bind IB and VB for quad rendering
    UINT strides[] = {sizeof(Float3)};
    UINT offsets[] = {0};
    pCtx->D3DContext->IASetVertexBuffers(0, 1, &pRenderer->screenQuadVB, strides, offsets);
    pCtx->D3DContext->IASetIndexBuffer(pRenderer->screenQuadIB, DXGI_FORMAT_R32_UINT, 0);

    pCtx->D3DContext->RSSetState(pRenderer->defaultRasterizerState);
    pCtx->D3DContext->OMSetDepthStencilState(pRenderer->defaultDepthStencilState, 0);

    //bind shaders
    pCtx->BindShader(&pRenderer->fullScreenQuadVS, 0);
    pCtx->BindShader(&mDirLightCS, 0);

    pCtx->D3DContext->DrawIndexed(6, 0, 0);
}

} // namespace Render
} // namespace NFE
