#pragma once

#include "../Renderers/RendererInterface/Device.hpp"

using namespace NFE;
using namespace NFE::Renderer;

class nfRendererTest : public testing::Test
{
protected:
    // preapre test case enviroment - initialize the renderer
    static void SetUpTestCase();

    // preapre test case enviroment - release the renderer
    static void TearDownTestCase();

    static HMODULE gRendererModule;
    static IDevice* gRendererDevice;
    static ICommandBuffer* gCommandBuffer;
};
