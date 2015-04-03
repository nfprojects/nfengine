#pragma once

#include "../Renderers/RendererInterface/Device.hpp"
#include "../nfCommon/Library.hpp"

using namespace NFE;
using namespace NFE::Renderer;

class RendererTest : public testing::Test
{
protected:
    // preapre test case enviroment - initialize the renderer
    static void SetUpTestCase();

    // preapre test case enviroment - release the renderer
    static void TearDownTestCase();

    // texture used as render target
    std::unique_ptr<ITexture> mTestTexture;
    std::unique_ptr<IRenderTarget> mTestRenderTarget;

    // texture used for GPU -> CPU data transfer
    std::unique_ptr<ITexture> mTestTextureRead;

    /**
     * Start rendering into test render target texture.
     * width,height      Texture dimensions
     * format, texelSize Texture format
     * @see EndFrame
     */
    bool BeginTestFrame(int width, int height, ElementFormat format, int texelSize);

    /**
     * Finish rendering into test render target texture and return rendered pixels buffer.
     * data[out] Buffer, where renderer pixels will be written into. Note that the buffer
                 size must be big enough to store all the pixels.
     * @see BeginFrame
     */
    bool EndTestFrame(void* data);

    static Common::Library gRendererLib;
    static IDevice* gRendererDevice;
    static ICommandBuffer* gCommandBuffer;
};
