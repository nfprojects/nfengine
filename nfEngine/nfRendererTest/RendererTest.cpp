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
