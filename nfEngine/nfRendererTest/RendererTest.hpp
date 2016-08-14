#pragma once

#include "../Renderers/RendererInterface/Device.hpp"
#include "nfCommon/Library.hpp"

using namespace NFE;
using namespace NFE::Renderer;

class RendererTest : public testing::Test
{
protected:
    // preapre test case enviroment - initialize the renderer
    static void SetUpTestCase();

    // preapre test case enviroment - release the renderer
    static void TearDownTestCase();

    static Common::Library gRendererLib;
    static IDevice* gRendererDevice;

public:
    ~RendererTest() { }
};
