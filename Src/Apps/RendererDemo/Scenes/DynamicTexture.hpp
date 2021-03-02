/**
 * @file
 * @author  Witek902
 * @brief   Declarations of Dynamic Texture scene
 */

#pragma once

#include "Scene.hpp"
#include "../../../Engine/Common/Math/Random.hpp"

class DynamicTextureScene : public Scene
{
    NFE::Renderer::TexturePtr mTexture;
    NFE::Renderer::RenderTargetPtr mRenderTarget;

    // using some weird resolution to make sure it works as intended
    static constexpr NFE::uint32 TexRegionWidth = 191;
    static constexpr NFE::uint32 TexRegionHeight = 233;

    bool mUseCopyQueue = false;

    NFE::Math::Random mRandom;
    NFE::Common::DynArray<NFE::uint32> mTextureData; // assumes RGBA 8-bit format

    float mTime = 0.0f;

    // Releases only subscene-related resources. Backbuffer, RT and BlendState stay intact.
    void ReleaseSubsceneResources() override;

    // Subscenes
    bool CreateSubSceneSimple(bool useCopyQueue);

public:
    DynamicTextureScene();
    ~DynamicTextureScene();

    bool OnInit(void* winHandle) override;
    void Draw(float dt) override;
    void Release() override;
};
