#include "PCH.hpp"
#include "Backends.hpp"
#include "RendererTest.hpp"

/// static members definitions
Common::Library RendererTest::gRendererLib;
IDevice* RendererTest::gRendererDevice = nullptr;
std::string RendererTest::gTestShaderPath;

void RendererTest::SetUpTestCase()
{
    ASSERT_TRUE(gRendererLib.Open(gBackend));

    RendererInitFunc proc;
    ASSERT_TRUE(gRendererLib.GetSymbol(RENDERER_INIT_FUNC, proc));
    ASSERT_TRUE(proc != nullptr);

    DeviceInitParams params;
    params.preferredCardId = gPreferedCardId;
    params.debugLayer = gUseDebugLayer;
    gRendererDevice = proc(&params);
    ASSERT_TRUE(gRendererDevice != nullptr);

    gTestShaderPath = gShaderPathPrefix + "Simple" + gShaderPathExt;
}

void RendererTest::TearDownTestCase()
{
    if (gRendererDevice != nullptr)
    {
        gRendererDevice = nullptr;
        RendererReleaseFunc proc;
        ASSERT_TRUE(gRendererLib.GetSymbol(RENDERER_RELEASE_FUNC, proc));
        ASSERT_TRUE(proc != nullptr);
        proc();
    }

    gRendererLib.Close();
}

void RendererTest::BeginTestFrame(int width, int height, ElementFormat format)
{
    mCommandBuffer.reset(gRendererDevice->CreateCommandBuffer());
    ASSERT_FALSE(!mCommandBuffer);

    mCommandBuffer->Reset();

    TextureDesc texDesc;
    texDesc.binding = NFE_RENDERER_TEXTURE_BIND_RENDERTARGET;
    texDesc.access = BufferAccess::GPU_ReadWrite;
    texDesc.format = format;
    texDesc.width = width;
    texDesc.height = height;
    mTestTexture.reset(gRendererDevice->CreateTexture(texDesc));
    ASSERT_FALSE(!mTestTexture);

    RenderTargetElement rtTarget;
    rtTarget.texture = mTestTexture.get();
    RenderTargetDesc rtDesc;
    rtDesc.numTargets = 1;
    rtDesc.targets = &rtTarget;
    mTestRenderTarget.reset(gRendererDevice->CreateRenderTarget(rtDesc));
    ASSERT_FALSE(!mTestRenderTarget);

    texDesc.binding = 0;
    texDesc.access = BufferAccess::CPU_Read;
    mTestTextureRead.reset(gRendererDevice->CreateTexture(texDesc));
    ASSERT_FALSE(!mTestTextureRead);

    mCommandBuffer->SetRenderTarget(mTestRenderTarget.get());
}

void RendererTest::EndTestFrame(void* data)
{
    mCommandBuffer->SetRenderTarget(nullptr);
    mCommandBuffer->CopyTexture(mTestTexture.get(), mTestTextureRead.get());

    gRendererDevice->Execute(mCommandBuffer->Finish().get());

    ASSERT_TRUE(gRendererDevice->DownloadTexture(mTestTextureRead.get(), data));

    mTestRenderTarget.reset();
    mTestTexture.reset();
    mTestTextureRead.reset();
}
