#pragma once

#include "RendererTest.hpp"

using namespace NFE;
using namespace NFE::Renderer;

class DrawTest : public RendererTest
{
    // texture used as render target
    struct TargetTexture
    {
        std::unique_ptr<const TexturePtr&>   texture;            // render target texture
        std::unique_ptr<const TexturePtr&>   readbackTexture;    // texture used for GPU -> CPU data transfer
        std::unique_ptr<char[]>     pixelData;
        size_t                      textureSize;
        ElementFormat               format;
    };

    RenderTargetPtr mTestRenderTarget;
    std::vector<TargetTexture> mTargetTextures;

    uint32 mTestTextureWidth;
    uint32 mTestTextureHeight;

protected:
    std::unique_ptr<ICommandRecorder> mCommandBuffer;

    /**
     * Start rendering into test render target texture.
     * @param width,height      Texture dimensions
     * @param numTargets        Number of render target textures
     * @param formats           Render target textures formats
     * @see EndFrame
     */
    void BeginTestFrame(uint32 width, uint32 height, size_t numTargets, ElementFormat* formats);

    /**
     * Finish rendering into test render target texture and read rendered pixels buffer.
     * Use VerifyPixelsXXX methods to verify textures content.
     * @see BeginFrame
     */
    void EndTestFrame();


    template<typename T>
    void VerifyPixelsInteger(size_t id, const T* expected)
    {
        // Graphics APIs assume that there can be some formats conversion tolerance
        const T maxError = 2;

        ASSERT_LT(id, mTargetTextures.size());
        const TargetTexture& target = mTargetTextures[id];
        uint32 channels = GetElementFormatChannels(target.format);
        const T* pixelData = reinterpret_cast<const T*>(target.pixelData.get());

        for (uint32 y = 0; y < mTestTextureHeight; ++y)
        {
            for (uint32 x = 0; x < mTestTextureWidth; ++x)
            {
                for (uint32 ch = 0; ch < channels; ++ch)
                {
                    const T expectedVal = expected[ch];
                    const T actualVal = pixelData[channels * (mTestTextureWidth * y + x) + ch];

                    ASSERT_NEAR(expectedVal, actualVal, maxError) <<
                        "Invalid pixel value: x=" + std::to_string(x) + ", y=" + std::to_string(y) +
                        ", channel=" + std::to_string(ch);
                }
            }
        }
    }

    void VerifyPixelsFloat(size_t id, const float* expected)
    {
        ASSERT_LT(id, mTargetTextures.size());
        const TargetTexture& target = mTargetTextures[id];
        uint32 channels = GetElementFormatChannels(target.format);
        const float* pixelData = reinterpret_cast<const float*>(target.pixelData.get());

        for (uint32 y = 0; y < mTestTextureHeight; ++y)
        {
            for (uint32 x = 0; x < mTestTextureWidth; ++x)
            {
                for (uint32 ch = 0; ch < channels; ++ch)
                {
                    ASSERT_NEAR(expected[ch],
                                pixelData[channels * (mTestTextureWidth * y + x) + ch],
                                NFE_MATH_EPSILON) <<
                        "Invalid pixel value: x=" + std::to_string(x) + ", y=" + std::to_string(y) +
                        ", channel=" + std::to_string(ch);
                }
            }
        }
    }

public:
    ~DrawTest() { }
};
