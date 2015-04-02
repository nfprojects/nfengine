#include "stdafx.hpp"
#include "RendererTest.hpp"

/// static members definitions
Common::Library RendererTest::gRendererLib;
IDevice* RendererTest::gRendererDevice = nullptr;
ICommandBuffer* RendererTest::gCommandBuffer = nullptr;

void RendererTest::SetUpTestCase()
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

void RendererTest::TearDownTestCase()
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
