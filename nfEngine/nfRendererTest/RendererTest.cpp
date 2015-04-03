#include "stdafx.hpp"
#include "RendererTest.hpp"

/// static members definitions
HMODULE nfRendererTest::gRendererModule = NULL;
IDevice* nfRendererTest::gRendererDevice = nullptr;
ICommandBuffer* nfRendererTest::gCommandBuffer = nullptr;


void nfRendererTest::SetUpTestCase()
{
    /*
     TODO:
     * portability
     * passing renderer name as the application parameter
     */

    gRendererModule = LoadLibrary(L"nfRendererD3D11.dll");
    ASSERT_TRUE(gRendererModule != NULL);

    auto proc = (RendererInitFunc)GetProcAddress(gRendererModule, RENDERER_INIT_FUNC);
    ASSERT_TRUE(proc != nullptr);

    gRendererDevice = proc();
    ASSERT_TRUE(gRendererDevice != nullptr);

    gCommandBuffer = gRendererDevice->GetDefaultCommandBuffer();
    ASSERT_TRUE(gCommandBuffer != nullptr);
}

void nfRendererTest::TearDownTestCase()
{
    if (gRendererDevice != nullptr)
    {
        gRendererDevice = nullptr;
        auto proc = (RendererReleaseFunc)GetProcAddress(gRendererModule, "Release");
        ASSERT_TRUE(proc != nullptr);
        proc();
    }
}

bool nfRendererTest::BeginTestFrame(int width, int height, ElementFormat format, int texelSize)
{
    TextureDesc texDesc;
    texDesc.binding = NFE_RENDERER_TEXTURE_BIND_RENDERTARGET;
    texDesc.access = BufferAccess::GPU_ReadWrite;
    texDesc.format = format;
    texDesc.texelSize = texelSize;
    texDesc.width = width;
    texDesc.height = height;
    mTestTexture.reset(gRendererDevice->CreateTexture(texDesc));
    if (!mTestTexture.get())
        return false;

    RenderTargetElement rtTarget;
    rtTarget.texture = mTestTexture.get();
    RenderTargetDesc rtDesc;
    rtDesc.numTargets = 1;
    rtDesc.targets = &rtTarget;
    mTestRenderTarget.reset(gRendererDevice->CreateRenderTarget(rtDesc));
    if (!mTestRenderTarget.get())
        return false;

    texDesc.binding = 0;
    texDesc.access = BufferAccess::CPU_Read;
    mTestTextureRead.reset(gRendererDevice->CreateTexture(texDesc));
    if (!mTestTextureRead.get())
        return false;

    gCommandBuffer->SetRenderTarget(mTestRenderTarget.get());

    return true;
}

bool nfRendererTest::EndTestFrame(void* data)
{
    gCommandBuffer->SetRenderTarget(nullptr);
    gCommandBuffer->CopyTexture(mTestTexture.get(), mTestTextureRead.get());
    bool success = gCommandBuffer->ReadTexture(mTestTextureRead.get(), data);

    mTestRenderTarget.reset();
    mTestTexture.reset();
    mTestTextureRead.reset();

    return success;
}