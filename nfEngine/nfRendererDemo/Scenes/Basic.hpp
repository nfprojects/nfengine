/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Declarations of Scene functions
 *
 * The subscenes described in this file are meant to help in new Renderer backend creation process.
 * In theory, each subscene should add a small amount of Renderers Interfaces (ideally, only one),
 * which will represent specific Renderers features. The subscenes should rely on each other - more
 * complex subscene will add new Interface initialization to simpler scenes. This way it can be
 * easily tracked which features are yet to be implemented in a newly developed Renderer backend.
 */

#pragma once

#include "Scene.hpp"

class BasicScene: public Scene
{
    // Renderer interfaces generated by BasicScene
    NFE::Renderer::RenderTargetPtr mWindowRenderTarget;
    NFE::Renderer::PipelineStatePtr mPipelineState;
    NFE::Renderer::ShaderPtr mVertexShader;
    NFE::Renderer::ShaderPtr mPixelShader;
    NFE::Renderer::BufferPtr mVertexBuffer;
    NFE::Renderer::VertexLayoutPtr mVertexLayout;
    NFE::Renderer::BufferPtr mIndexBuffer;
    NFE::Renderer::BufferPtr mConstantBuffer;
    NFE::Renderer::SamplerPtr mSampler;
    NFE::Renderer::TexturePtr mTexture;

    NFE::Renderer::ResourceBindingSetPtr mVSBindingSet;
    NFE::Renderer::ResourceBindingSetPtr mPSBindingSet;
    NFE::Renderer::ResourceBindingLayoutPtr mResBindingLayout;
    NFE::Renderer::ResourceBindingInstancePtr mVSBindingInstance;
    NFE::Renderer::ResourceBindingInstancePtr mPSBindingInstance;

    int mTextureSlot;
    int mCBufferSlot;

    // Used for objects rotation in Constant Buffer scenes and onward
    float mAngle;

    // how many instances will be drawn?
    int mGridSize;

    int mVSBindingSlot;
    int mPSBindingSlot;

    // cbuffer mode
    NFE::Renderer::BufferMode mCBufferMode;

    // Releases only subscene-related resources. Backbuffer, RT and BlendState stay intact.
    void ReleaseSubsceneResources() override;

    // Resource creators for subscenes
    bool CreateShaders(bool useCBuffer, bool useTexture, NFE::Renderer::BufferMode cbufferMode);
    bool CreateVertexBuffer(bool withExtraVert);
    bool CreateIndexBuffer();
    bool CreateConstantBuffer(NFE::Renderer::BufferMode cbufferMode);
    bool CreateTexture();
    bool CreateSampler();

    // Subscenes
    bool CreateSubSceneEmpty();
    bool CreateSubSceneVertexBuffer();
    bool CreateSubSceneIndexBuffer();
    bool CreateSubSceneConstantBuffer(NFE::Renderer::BufferMode cbufferMode);
    bool CreateSubSceneTexture(NFE::Renderer::BufferMode cbufferMode, int gridSize = 1);

public:
    BasicScene();
    ~BasicScene();

    bool OnInit(void* winHandle) override;
    void Draw(float dt) override;
    void Release() override;
};
