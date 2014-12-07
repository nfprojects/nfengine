#include "stdafx.hpp"
#include "Renderer.hpp"
#include "../nfCommon/Image.hpp"
#include "../nfCommon/Logger.hpp"

#include "GeometryBufferRenderer.hpp"
#include "ShadowsRenderer.hpp"
#include "LightsRenderer.hpp"
#include "DebugRenderer.hpp"
#include "PostProcessRenderer.hpp"
#include "GuiRenderer.hpp"

namespace NFE {
namespace Render {

GPUStats::GPUStats()
{
    IAVertices = 0;
    IAPrimitives = 0;
    VSInvocations = 0;
    GSInvocations = 0;
    GSPrimitives = 0;
    CInvocations = 0;
    CPrimitives = 0;
    PSInvocations = 0;
    HSInvocations = 0;
    DSInvocations = 0;
    CSInvocations = 0;
}


RendererD3D11::RendererD3D11()
{
    // TODO: use smart pointers
    DXGIFactory = 0;
    D3DDevice = 0;
    statsQuery = 0;

    defaultBlendState = 0;
    defaultDepthStencilState = 0;
    defaultRasterizerState = 0;
    defaultSampler = 0;

    depthStencilBuffer = 0;
    depthStencilView = 0;
    depthStencilSRV = 0;

    defaultDiffuseTexture = 0;
    defaultNormalTexture = 0;
    defaultSpecularTexture = 0;
    noiseTexture = 0;

    screenQuadVB = 0;
    screenQuadIB = 0;
    fullScreenQuadIL = 0;

    mImmContext = nullptr;
    mPostProcessRenderer = nullptr;
    mGuiRenderer = nullptr;
    mLightRenderer = nullptr;
    mDebugRenderer = nullptr;
    mGBufferRenderer = nullptr;
    mShadowRenderer = nullptr;

    bufferWidth = bufferHeight = 0;
}

RendererD3D11::~RendererD3D11()
{
    D3D_SAFE_RELEASE(screenQuadVB);
    D3D_SAFE_RELEASE(screenQuadIB);
    D3D_SAFE_RELEASE(fullScreenQuadIL);
    fullScreenQuadVS.Release();

    delete defaultDiffuseTexture;
    delete defaultNormalTexture;
    delete defaultSpecularTexture;
    delete noiseTexture;
    defaultDiffuseTexture = 0;
    defaultNormalTexture = 0;
    defaultSpecularTexture = 0;
    noiseTexture = 0;

    geomBuffer.Release();
    renderTarget.Release();
    postProcessed.Release();
    bloomA.Release();
    bloomB.Release();

    ReleaseModules();

    LOG_INFO("Releasing renderer...");

    D3D_SAFE_RELEASE(depthStencilView);
    D3D_SAFE_RELEASE(depthStencilSRV);
    D3D_SAFE_RELEASE(depthStencilBuffer);

    D3D_SAFE_RELEASE(defaultBlendState);
    D3D_SAFE_RELEASE(defaultDepthStencilState);
    D3D_SAFE_RELEASE(defaultRasterizerState);
    D3D_SAFE_RELEASE(defaultSampler);

    D3D_SAFE_RELEASE(statsQuery);
    D3D_SAFE_RELEASE(DXGIFactory);
    D3D_SAFE_RELEASE(D3DDevice);
}

void* RendererD3D11::GetDevice() const
{
    return (void*)D3DDevice;
}

RendererD3D11*& RendererD3D11::GetInstance()
{
    static RendererD3D11* pRenderer = NULL;
    return pRenderer;
}

int RendererD3D11::Init()
{
    HRESULT hr;

    // create a device, device context and swap chain using the information in the scd struct
    UINT flags = 0;

#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    //create Direct3D Device
    D3D_FEATURE_LEVEL pLevels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};
    ID3D11DeviceContext* pD3DImmediateContext;


    hr = D3D_CHECK(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, pLevels, 3,
                                     D3D11_SDK_VERSION, \
                                     &D3DDevice, &featureLevel, &pD3DImmediateContext));

    if (FAILED(hr))
    {
        LOG_FATAL("Could not create Direct3D device!");
        return 1;
    }


    // create immediate rendering context
    mImmContext = new RenderContextD3D11;
    mImmContext->Init(0, this, pD3DImmediateContext);


    //log feature level used
    featureLevel = D3DDevice->GetFeatureLevel();
    char levelStr[8];
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_0:
            strcpy_s(levelStr, "11");
            break;

        case D3D_FEATURE_LEVEL_10_1:
            strcpy_s(levelStr, "10.1");
            break;

        case D3D_FEATURE_LEVEL_10_0:
            strcpy_s(levelStr, "10.0");
            break;
    }
    LOG_INFO("Direct3D device created with feature level %s.", levelStr);


    // check features support
    D3DDevice->CheckFeatureSupport(D3D11_FEATURE_DOUBLES, &gpuFeatures.doubles,
                                   sizeof(gpuFeatures.doubles));
    D3DDevice->CheckFeatureSupport(D3D11_FEATURE_THREADING, &gpuFeatures.threading,
                                   sizeof(gpuFeatures.threading));
    D3DDevice->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS,
                                   &gpuFeatures.D3D10Options, sizeof(gpuFeatures.D3D10Options));

    // log GPU features
    std::string featureStr;
    if (gpuFeatures.doubles.DoublePrecisionFloatShaderOps) featureStr += "Doubles, ";
    if (gpuFeatures.threading.DriverCommandLists) featureStr += "DriverCommandLists, ";
    if (gpuFeatures.threading.DriverConcurrentCreates) featureStr += "DriverConcurrentCreates, ";
    if (gpuFeatures.D3D10Options.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x)
        featureStr += "ComputeShadersViaShaderModel4_x, ";
    LOG_INFO("GPU Supported features: %s", featureStr.c_str());


    //get DXGI factory for created Direct3D device
    IDXGIDevice* pDXGIDevice = 0;
    hr = D3DDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);
    IDXGIAdapter* pDXGIAdapter = 0;
    hr = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pDXGIAdapter);
    pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&DXGIFactory);

    //log GPU information
    DXGI_ADAPTER_DESC adapterDesc;
    pDXGIAdapter->GetDesc(&adapterDesc);
    LOG_INFO("GPU Description: %ws", adapterDesc.Description);
    LOG_INFO("GPU Vendor ID: %u", adapterDesc.VendorId);
    LOG_INFO("GPU Device ID: %u", adapterDesc.DeviceId);
    LOG_INFO("GPU Revision: %u", adapterDesc.Revision);
    LOG_INFO("GPU Dedicated Video Memory: %u MB", adapterDesc.DedicatedVideoMemory >> 20);
    LOG_INFO("GPU Dedicated System Memory: %u MB", adapterDesc.DedicatedSystemMemory >> 20);
    LOG_INFO("GPU Shared System Memory: %u MB", adapterDesc.SharedSystemMemory >> 20);

    D3D_SAFE_RELEASE(pDXGIAdapter);
    D3D_SAFE_RELEASE(pDXGIDevice);


    //create geoemtry buffer for deferred shading and HDR render tatgets
    bufferWidth = 1920;
    bufferHeight = 1080; //TODO: this should be changed at runtime!!!
    geomBuffer.Resize(bufferWidth, bufferHeight);
    renderTarget.Init(bufferWidth, bufferHeight, NULL, true);
    postProcessed.Init(bufferWidth, bufferHeight, NULL, true);
    toneMapped.Init(bufferWidth, bufferHeight, NULL, true);
    bloomA.Init(bufferWidth / 4, bufferHeight / 4, NULL, true);
    bloomB.Init(bufferWidth / 4, bufferHeight / 4, NULL, true);
    InitDepthBuffer();


    D3D11_BLEND_DESC blendDesc;
    blendDesc.AlphaToCoverageEnable = 0;
    blendDesc.IndependentBlendEnable = 0;
    blendDesc.RenderTarget[0].BlendEnable = 0;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    D3D_CHECK(D3DDevice->CreateBlendState(&blendDesc, &defaultBlendState));


    // Initialize default depth stencil state
    RendererD3D11DepthStencilDesc depthStencilDesc(TRUE, D3D11_DEPTH_WRITE_MASK_ZERO,
            D3D11_COMPARISON_ALWAYS);
    D3D_CHECK(D3DDevice->CreateDepthStencilState(&depthStencilDesc, &defaultDepthStencilState));

    // Initialize default sampler state
    RendererD3D11SamplerDesc samplerDesc(D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR);
    D3D_CHECK(D3DDevice->CreateSamplerState(&samplerDesc, &defaultSampler));


    // Initialize default rasterizer state
    D3D11_RASTERIZER_DESC rd;
    ZeroMemory(&rd, sizeof(rd));
    rd.CullMode = D3D11_CULL_NONE;
    rd.FrontCounterClockwise = 1;
    rd.FillMode = D3D11_FILL_SOLID;
    rd.DepthClipEnable = 0;
    D3D_CHECK(D3DDevice->CreateRasterizerState(&rd, &defaultRasterizerState));

    //pipeline statistics query
    D3D11_QUERY_DESC queryDesc;
    queryDesc.MiscFlags = 0;
    queryDesc.Query = D3D11_QUERY_PIPELINE_STATISTICS;
    D3D_CHECK(D3DDevice->CreateQuery(&queryDesc, &statsQuery));



    D3D11_SUBRESOURCE_DATA initData;
    D3D11_BUFFER_DESC bd;
    bd.MiscFlags = 0;

    //full screen quad vertex buffer
    Float3 pVerticies[] =
    {
        Float3( -1.0f,  -1.0f,  0.0f),
        Float3( 1.0f,  -1.0f,  0.0f),
        Float3( 1.0f,  1.0f,  0.0f),
        Float3( -1.0f,  1.0f,  0.0f),
    };
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.ByteWidth = 4 * sizeof(Float3);
    bd.CPUAccessFlags = 0;
    bd.Usage = D3D11_USAGE_IMMUTABLE;
    bd.StructureByteStride = sizeof(Float3);
    initData.pSysMem = pVerticies;
    initData.SysMemPitch = 0;
    initData.SysMemSlicePitch = 0;
    D3D_CHECK(D3DDevice->CreateBuffer(&bd, &initData, &screenQuadVB));

    //full screen quad index buffer
    uint32 pIndicies[] = {0, 1, 2, 0, 2, 3};
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.ByteWidth = 6 * sizeof(uint32);
    bd.CPUAccessFlags = 0;
    bd.Usage = D3D11_USAGE_IMMUTABLE;
    bd.StructureByteStride = sizeof(uint32);
    initData.pSysMem = pIndicies;
    D3D_CHECK(D3DDevice->CreateBuffer(&bd, &initData, &screenQuadIB));


    //create shader & input layout for full screen quad rendering
    fullScreenQuadVS.Init(this, ShaderType::Vertex, "FullScreenQuadVS");
    const D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    //init Input Layout
    Common::Buffer* pShaderSource = fullScreenQuadVS.GetShaderBytecode(0);
    D3D_CHECK(D3DDevice->CreateInputLayout(layout, 1, pShaderSource->GetData(),
                                           pShaderSource->GetSize(), &fullScreenQuadIL));

    //make sure that shaders' temporary directories exist
    CreateDirectoryA("Logs/DisassembledShaders", 0);
    CreateDirectoryA("Logs/ShadersCompilation", 0);
    CreateDirectoryA("ShaderCache", 0);
    CreateDirectoryA("ShaderCache_Debug", 0);

    if (InitModules() != 0)
    {
        LOG_FATAL("Renderer could not be initialized!");
        return 1;
    }

    Common::Image image;
    float diffuseColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    image.SetData(diffuseColor, 1, 1, Common::ImageFormat::RGBA_FLOAT);
    //defaultDiffuseTexture = CreateTexture(&image, 0);
    defaultDiffuseTexture = new RendererTextureD3D11;
    defaultDiffuseTexture->FromImage(image);

    float normalColor[] = {0.5f, 0.5f, 1.0f, 1.0f};
    image.SetData(normalColor, 1, 1, Common::ImageFormat::RGBA_FLOAT);
    //defaultNormalTexture = CreateTexture(&image, 0);
    defaultNormalTexture = new RendererTextureD3D11;
    defaultNormalTexture->FromImage(image);

    float specularColor[] = {0.1f, 0.1f, 0.1f, 0.0f};
    image.SetData(specularColor, 1, 1, Common::ImageFormat::RGBA_FLOAT);
    //defaultSpecularTexture = CreateTexture(&image, 0);
    defaultSpecularTexture = new RendererTextureD3D11;
    defaultSpecularTexture->FromImage(image);


    UCHAR* pData = (UCHAR*)malloc(256 * 256);
    for (int i = 0; i < 256 * 256; i++)
    {
        pData[i] = rand() % 256;
    }
    image.SetData(pData, 256, 256, Common::ImageFormat::A_UBYTE);
    noiseTexture = new RendererTextureD3D11;
    noiseTexture->FromImage(image);
//  noiseTexture = CreateTexture(&image, 0);
    free(pData);


    //reset query results
    pD3DImmediateContext->Begin(statsQuery);
    pD3DImmediateContext->End(statsQuery);

    return 0;
}

int RendererD3D11::InitModules()
{
    /*
        TODO:
        * Make it multithreaded.
        * Error checking.
    */
    mGBufferRenderer = new GBufferRendererD3D11;
    mGBufferRenderer->Init(this);

    mLightRenderer = new LightsRendererD3D11;
    mLightRenderer->Init(this);

    mShadowRenderer = new ShadowRendererD3D11;
    mShadowRenderer->Init(this);

    mDebugRenderer = new DebugRendererD3D11;
    mDebugRenderer->Init(this);

    mPostProcessRenderer = new PostProcessRendererD3D11;
    mPostProcessRenderer->Init(this);

    mGuiRenderer = new GuiRendererD3D11;
    mGuiRenderer->Init(this);

    return 0;
}

void RendererD3D11::ReleaseModules()
{
    LOG_INFO("Releasing rendering modules...");

    mGBufferRenderer->Release();
    mLightRenderer->Release();
    mShadowRenderer->Release();
    mDebugRenderer->Release();
    mPostProcessRenderer->Release();
    mGuiRenderer->Release();

    delete mGBufferRenderer;
    delete mLightRenderer;
    delete mShadowRenderer;
    delete mDebugRenderer;
    delete mPostProcessRenderer;
    delete mGuiRenderer;

    mGBufferRenderer = NULL;
    mLightRenderer = NULL;
    mShadowRenderer = NULL;
    mDebugRenderer = NULL;
    mPostProcessRenderer = NULL;
    mGuiRenderer = NULL;
}

void RendererD3D11::InitDepthBuffer()
{
    D3D_SAFE_RELEASE(depthStencilBuffer);
    D3D_SAFE_RELEASE(depthStencilView);

    HRESULT HR;

    // Set up the description of the depth buffer.
    D3D11_TEXTURE2D_DESC depthBufferDesc;
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
    depthBufferDesc.Width = bufferWidth;
    depthBufferDesc.Height = bufferHeight;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags = 0;

    // Create the texture for the depth buffer using the filled out description.
    HR = D3D_CHECK(D3DDevice->CreateTexture2D(&depthBufferDesc, NULL, &depthStencilBuffer));
    if (FAILED(HR))
        return;


    // Set up the depth stencil view description.
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
    depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // Create the depth stencil view.
    HR = D3D_CHECK(D3DDevice->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc,
                   &depthStencilView));
    if (FAILED(HR))
    {
        D3D_SAFE_RELEASE(depthStencilBuffer);
        return;
    }


    // Set up the shader resource view description.
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    // Create the shader resource view for depth map
    HR = D3D_CHECK(D3DDevice->CreateShaderResourceView(depthStencilBuffer, &srvDesc,
                   &depthStencilSRV));
    if (FAILED(HR))
    {
        D3D_SAFE_RELEASE(depthStencilBuffer);
        return;
    }
}


float saturate(float x)
{
    if (x > 1.0f) return 1.0f;
    if (x < 0.0f) return 0.0f;
    return x;
}

float smoothstep(float edge0, float edge1, float x)
{
    x = saturate((x - edge0) / (edge1 - edge0));
    return x * x * x * (x * (x * 6.0f - 15.0f) + 10.0f);
}

float smoothclamp(float edge0, float edge1, float x)
{
    return edge0 + (edge1 - edge0) * smoothstep(edge0, edge1, x);
}


void RendererD3D11::Begin()
{
    //query pipeline statistics
    if (settings.pipelineStats)
    {
        mImmContext->D3DContext->End(statsQuery);

        D3D11_QUERY_DATA_PIPELINE_STATISTICS data;

        while (mImmContext->D3DContext->GetData(statsQuery, &data, sizeof(data), 0) == S_FALSE) {};
        mImmContext->D3DContext->Begin(statsQuery);

        pipelineStats.CInvocations = (uint32)data.CInvocations;
        pipelineStats.CPrimitives = (uint32)data.CPrimitives;
        pipelineStats.CSInvocations = (uint32)data.CSInvocations;
        pipelineStats.VSInvocations = (uint32)data.VSInvocations;
        pipelineStats.GSInvocations = (uint32)data.GSInvocations;
        pipelineStats.HSInvocations = (uint32)data.HSInvocations;
        pipelineStats.DSInvocations = (uint32)data.DSInvocations;
        pipelineStats.PSInvocations = (uint32)data.PSInvocations;
        pipelineStats.IAPrimitives = (uint32)data.IAPrimitives;
        pipelineStats.IAVertices = (uint32)data.IAVertices;
        pipelineStats.GSPrimitives = (uint32)data.GSPrimitives;
    }
}

//calculate delta time text color, based on value
uint32 DeltaTimeColor(double dt)
{
    double points[] = {0.0, 16.6666, 33.333, 66.6667};
    float colorsR[] = {1.0f, 0.0f, 0.75f, 1.0f};
    float colorsG[] = {1.0f, 1.0f, 0.5f, 0.0f};
    float colorsB[] = {1.0f, 0.0f, 0.1f, 0.0f};

    float color[3];
    color[0] = colorsR[3];
    color[1] = colorsG[3];
    color[2] = colorsB[3];

    for (int i = 1; i < 4; i++)
    {
        if (dt < points[i])
        {
            double x = (dt - points[i - 1]) / (points[i] - points[i - 1]);

            //interpolate color
            color[0] = colorsR[i - 1] * (float)(1.0 - x) + colorsR[i] * (float)x;
            color[1] = colorsG[i - 1] * (float)(1.0 - x) + colorsG[i] * (float)x;
            color[2] = colorsB[i - 1] * (float)(1.0 - x) + colorsB[i] * (float)x;
            break;
        }
    }

    uint32 result = 0xFF000000;
    result |= (uint32)(color[0] * 255.0f);
    result |= (uint32)(color[1] * 255.0f) << 8;
    result |= (uint32)(color[2] * 255.0f) << 16;
    return result;
}

void RendererD3D11::SwapBuffers(IRenderTarget* pRenderTarget, ViewSettings* pViewSettings, float dt)
{
    IRenderContext* pCtx = mImmContext;
    RenderTargetD3D11* pRT = dynamic_cast<RenderTargetD3D11*>(pRenderTarget);
    if (!pRT) return;

    mPostProcessRenderer->Enter(pCtx);

    uint32 width = pRT->width;
    uint32 height = pRT->height;

    //motion blur
    RenderTargetD3D11* pToneMapped = &toneMapped;
    RenderTargetD3D11* pPostProcessed = &renderTarget;
    if (settings.motionBlur)
    {
        //pCtx->D3DContext->GenerateMips(renderTarget.SRV);

        float motionBlurFactor = dt;
        if (motionBlurFactor <= 0.016666f) //vsync frequency
            motionBlurFactor = 0.016666f;

        motionBlurFactor *= settings.motionBlurFactor;

        pPostProcessed = &postProcessed;
        mPostProcessRenderer->ApplyMotionBlur(pCtx, width, height, &renderTarget, pPostProcessed,
                                              motionBlurFactor);
    }


    //downsample (if needed)
    if (settings.bloom || settings.autoExposure)
        mPostProcessRenderer->Downsaple(pCtx, width, height, pPostProcessed, &bloomA);

    //get average brightness of rendered image
    float avarageColor = 0.5f;
    if (settings.autoExposure)
    {
        // TODO: exposure should be computed on GPU. Fetching average color to the CPU is slow
        mPostProcessRenderer->AverageTexture(pCtx, &bloomA, width / 4, height / 4);
        avarageColor = mPostProcessRenderer->GetAverageColor(pCtx, width / 4, height / 4);
    }

    //create bloom texture
    if (settings.bloom)
    {
        mPostProcessRenderer->Blur(pCtx, width / 4, height / 4, &bloomA, &bloomB, 0);
        mPostProcessRenderer->Blur(pCtx, width / 4, height / 4, &bloomB, &bloomA, 1);
    }

    const float minExposure = 0.5f;
    const float maxExposure = 4.0f;

    float factor = 0.0f;
    float CurrExposure = 0.0f;

    if (settings.autoExposure)
    {
        /*
        // LOGARITHMIC - works well with 1-exp(-e*c) tone mapping
        float DestExposure;
        if (avarageColor < 0.0001f)
        {
            DestExposure = maxExposure;
        }
        else
        {
            //exposure that produce final image of average color equal to 0.5
            DestExposure = -log(0.25f) / avarageColor;
            DestExposure = smoothclamp(minExposure, maxExposure, DestExposure);
        }

        float CurrExposure = pDisplay->exposure;

        //low pass filter - for smooth exposure transition (in logarithmic scale)
        const float inertia = 1.3f;
        float factor = dt / (inertia + dt);
        float Tmp = (1.0f-factor) * logf(CurrExposure) + factor * logf(DestExposure);
        pDisplay->exposure = expf(Tmp);
        */

        // LINEAR - works better with filimc tone mapping
        float DestExposure;
        if (avarageColor < 0.0001f)
        {
            DestExposure = maxExposure;
        }
        else if (_isnan(avarageColor) || !_finite(avarageColor))
        {
            DestExposure = minExposure;
            MessageBoxA(0, "wtf?", "", 0);
        }
        else
        {
            //exposure that produce final image of average color equal to 0.5
            DestExposure = 0.2f / avarageColor;
            DestExposure = smoothclamp(minExposure, maxExposure, DestExposure);
        }

        CurrExposure = pViewSettings->exposure;

        //low pass filter - for smooth exposure transition (in logarithmic scale)
        const float inertia = 1.3f;
        factor = dt / (inertia + dt);
        float Tmp = (1.0f - factor) * CurrExposure + factor * DestExposure;
        pViewSettings->exposure = Tmp;


        if (pViewSettings->exposure > maxExposure) pViewSettings->exposure = maxExposure;
        if (pViewSettings->exposure < minExposure) pViewSettings->exposure = minExposure;
    }



    //clamp to min/max values
    //pDisplay->clampedExposure = smoothclamp(minExposure, maxExposure, pDisplay->exposure);

    bool antialiasingEnabled = settings.antialiasing;
    bool noiseEnabled = settings.noiseEnabled && settings.noiseLevel > 0.005;

    ToneMappingDesc toneMappingDesc;
    toneMappingDesc.exposure = pViewSettings->exposure;
    toneMappingDesc.pDest = antialiasingEnabled ? pToneMapped : pRT;
    toneMappingDesc.pSource = pPostProcessed;
    toneMappingDesc.pBloom = settings.bloom ? &bloomA : 0;
    toneMappingDesc.bloomFactor = settings.bloomFactor;
    toneMappingDesc.saturation = settings.saturation;
    toneMappingDesc.noiseLevel = settings.noiseLevel;
    toneMappingDesc.noiseEnabled = noiseEnabled && !antialiasingEnabled;
    mPostProcessRenderer->ApplyTonemapping(pCtx, toneMappingDesc);

    if (antialiasingEnabled)
    {
        FXAADesc fxaa;
        fxaa.pDest = pRT;
        fxaa.pSource = pToneMapped;
        fxaa.noiseLevel = settings.noiseLevel;
        fxaa.noiseEnabled = noiseEnabled;
        fxaa.lumaOpt = settings.antialiasingLumaOpt;
        fxaa.spanMax = settings.antialiasingSpanMax;
        fxaa.reduceMul = settings.antialiasingReduceMul;
        fxaa.reduceMin = settings.antialiasingReduceMin;
        mPostProcessRenderer->ApplyFXAA(pCtx, fxaa);
    }

    mPostProcessRenderer->Leave(pCtx);
}

void RendererD3D11::ExecuteDeferredContext(IRenderContext* pContext)
{
    auto pCtx = (RenderContextD3D11*)pContext;

    if (!pCtx)
        return;

    if (pCtx->commandList)
    {
        ((RenderContextD3D11*)mImmContext)->D3DContext->ExecuteCommandList(pCtx->commandList, 0);
        D3D_SAFE_RELEASE(pCtx->commandList);
    }
}


IGuiRenderer* RendererD3D11::GetGuiRenderer() const
{
    return mGuiRenderer;
}

IPostProcessRenderer* RendererD3D11::GetPostProcessRenderer() const
{
    return mPostProcessRenderer;
}

IDebugRenderer* RendererD3D11::GetDebugRenderer() const
{
    return mDebugRenderer;
}

IShadowRenderer* RendererD3D11::GetShadowRenderer() const
{
    return mShadowRenderer;
}

IGBufferRenderer* RendererD3D11::GetGBufferRenderer() const
{
    return mGBufferRenderer;
}

ILightsRenderer* RendererD3D11::GetLightsRenderer() const
{
    return mLightRenderer;
}


IRenderContext* RendererD3D11::GetImmediateContext() const
{
    return mImmContext;
}

IRenderContext* RendererD3D11::CreateDeferredContext()
{
    RenderContextD3D11* pContext = new RenderContextD3D11;
    pContext->Init(1, this);

    return pContext;
}

IRendererBuffer* RendererD3D11::CreateBuffer()
{
    // TODO: logging and error checking
    return new RendererBufferD3D11();
}

IRendererTexture* RendererD3D11::CreateTexture()
{
    // TODO: logging and error checking
    return new RendererTextureD3D11();
}

IRenderTarget* RendererD3D11::CreateRenderTarget()
{
    // TODO: logging and error checking
    return new RenderTargetD3D11();
}

IShadowMap* RendererD3D11::CreateShadowMap()
{
    // TODO: logging and error checking
    return new ShadowMapD3D11();
}

} // namespace Render
} // namespace NFE
