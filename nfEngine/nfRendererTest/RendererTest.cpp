#include "stdafx.hpp"
#include "RendererTest.hpp"

/// static members definitions
Common::Library nfRendererTest::gRendererLib;
IDevice* nfRendererTest::gRendererDevice = nullptr;
ICommandBuffer* nfRendererTest::gCommandBuffer = nullptr;

void nfRendererTest::SetUpTestCase()
{
    /*
     TODO:
     * passing renderer name as the application parameter
     */

    ASSERT_TRUE(gRendererLib.Open("nfRendererD3D11.dll"));

    auto proc = static_cast<RendererInitFunc>(gRendererLib.GetSymbol(RENDERER_INIT_FUNC));
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
        auto proc = static_cast<RendererReleaseFunc>(gRendererLib.GetSymbol("Release"));
        ASSERT_TRUE(proc != nullptr);
        proc();
    }

    gRendererLib.Close();
}
