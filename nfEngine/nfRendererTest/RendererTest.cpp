#include "PCH.hpp"
#include "Backends.hpp"
#include "RendererTest.hpp"

/// static members definitions
Common::Library RendererTest::gRendererLib;
IDevice* RendererTest::gRendererDevice = nullptr;

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