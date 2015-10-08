#include "PCH.hpp"
#include "Backends.hpp"
#include "RendererTest.hpp"

/// static members definitions
Common::Library RendererTest::gRendererLib;
IDevice* RendererTest::gRendererDevice = nullptr;
ICommandBuffer* RendererTest::gCommandBuffer = nullptr;
std::string RendererTest::gTestShaderPath;

void RendererTest::SetUpTestCase()
{
    ASSERT_TRUE(gRendererLib.Open(gBackend));

    RendererInitFunc proc;
    ASSERT_TRUE(gRendererLib.GetSymbol(RENDERER_INIT_FUNC, proc));
    ASSERT_TRUE(proc != nullptr);

    gRendererDevice = proc();
    ASSERT_TRUE(gRendererDevice != nullptr);

    gCommandBuffer = gRendererDevice->GetDefaultCommandBuffer();
    ASSERT_TRUE(gCommandBuffer != nullptr);

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

bool RendererTest::BeginTestFrame(int width, int height, ElementFormat format, int texelSize)
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

bool RendererTest::EndTestFrame(void* data)
{
    gCommandBuffer->SetRenderTarget(nullptr);
    //gCommandBuffer->CopyTexture(mTestTexture.get(), mTestTextureRead.get());
    bool success = gCommandBuffer->ReadTexture(mTestTexture.get(), data);

    mTestRenderTarget.reset();
    mTestTexture.reset();
    mTestTextureRead.reset();

    return success;
}
