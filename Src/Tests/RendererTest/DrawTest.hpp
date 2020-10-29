#pragma once

#include "RendererTest.hpp"

#include "Engine/Common/Math/Math.hpp"


using namespace NFE;
using namespace NFE::Renderer;

class DrawTest : public RendererTest
{
    // texture used as render target
    struct TargetTexture
    {
        TexturePtr texture;            // render target texture
        Common::UniquePtr<uint8[]> pixelData;
        size_t textureSize;
        size_t textureRowPitch;
        Format format;
    };

    RenderTargetPtr mTestRenderTarget;
    Common::DynArray<TargetTexture> mTargetTextures;

    uint32 mTestTextureWidth;
    uint32 mTestTextureHeight;

protected:
    CommandRecorderPtr mCommandBuffer;

    /**
     * Start rendering into test render target texture.
     * @param width,height      Texture dimensions
     * @param numTargets        Number of render target textures
     * @param formats           Render target textures formats
     * @see EndFrame
     */
    void BeginTestFrame(uint32 width, uint32 height, uint32 numTargets, const Format* formats);

    /**
     * Finish rendering into test render target texture and read rendered pixels buffer.
     * Use VerifyPixelsXXX methods to verify textures content.
     * @see BeginFrame
     */
    void EndTestFrame();


    template<typename T>
    void VerifyPixelsInteger(uint32 id, const T* expected)
    {
        // Graphics APIs assume that there can be some formats conversion tolerance
        const T maxError = 2;

        const TargetTexture& target = mTargetTextures[id];
        uint32 channels = GetElementFormatChannels(target.format);
        const T* pixelData = reinterpret_cast<const T*>(target.pixelData.Get());

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

    void VerifyPixelsFloat(uint32 id, const float* expected)
    {
        const TargetTexture& target = mTargetTextures[id];
        uint32 channels = GetElementFormatChannels(target.format);
        const float* pixelData = reinterpret_cast<const float*>(target.pixelData.Get());

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
