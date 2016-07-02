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

    std::unique_ptr<ICommandBuffer> mCommandBuffer;

    // texture used as render target
    std::unique_ptr<ITexture> mTestTexture;
    std::unique_ptr<IRenderTarget> mTestRenderTarget;

    // texture used for GPU -> CPU data transfer
    std::unique_ptr<ITexture> mTestTextureRead;

    /**
     * Start rendering into test render target texture.
     * @param width,height      Texture dimensions
     * @param format, texelSize Texture format
     * @see EndFrame
     */
    void BeginTestFrame(int width, int height, ElementFormat format, int texelSize);

    /**
     * Finish rendering into test render target texture and return rendered pixels buffer.
     * @param data[out] Buffer, where renderer pixels will be written into. Note that the buffer
     *                  size must be big enough to store all the pixels.
     * @see BeginFrame
     */
    void EndTestFrame(void* data);

    static Common::Library gRendererLib;
    static IDevice* gRendererDevice;
    static std::string gTestShaderPath;
};
