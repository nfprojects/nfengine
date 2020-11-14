#include "PCH.hpp"
#include "Backends.hpp"
#include "RendererTest.hpp"

/// static members definitions
Common::Library RendererTest::gRendererLib;
IDevice* RendererTest::gRendererDevice = nullptr;
CommandQueuePtr RendererTest::gMainCommandQueue;
CommandQueuePtr RendererTest::gComputeCommandQueue;
CommandQueuePtr RendererTest::gCopyCommandQueue;

void RendererTest::SetUpTestCase()
{
    ASSERT_TRUE(gRendererLib.Open(gBackend));

    RendererInitFunc proc;
    ASSERT_TRUE(gRendererLib.GetSymbol(RENDERER_INIT_FUNC, proc));
    ASSERT_TRUE(proc != nullptr);

    DeviceInitParams params;
    params.preferredCardId = gPreferedCardId;
    params.debugLevel = gDebugLevel;
    gRendererDevice = proc(&params);
    ASSERT_TRUE(gRendererDevice != nullptr);

    gMainCommandQueue = gRendererDevice->CreateCommandQueue(CommandQueueType::Graphics, "Main");
    gComputeCommandQueue = gRendererDevice->CreateCommandQueue(CommandQueueType::Compute, "Compute");
    gCopyCommandQueue = gRendererDevice->CreateCommandQueue(CommandQueueType::Copy, "Copy");
}

void RendererTest::TearDownTestCase()
{
    gMainCommandQueue.Reset();
    gComputeCommandQueue.Reset();
    gCopyCommandQueue.Reset();

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