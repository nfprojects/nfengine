/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Header file with multithreaded test scene declaration
 */

#pragma once

#include "Scene.hpp"

#include "nfCommon/Utils/ThreadPool.hpp"


class MultithreadedScene : public Scene
{
    using CollectedCommandLists = std::vector<NFE::Renderer::CommandListID>;

    std::vector<CollectedCommandLists> mCollectedCommandLists; // separate for each thread
    std::vector<NFE::Renderer::CommandRecorderPtr> mCommandRecorders;

    NFE::Renderer::RenderTargetPtr mWindowRenderTarget;
    NFE::Renderer::PipelineStatePtr mPipelineState;
    NFE::Renderer::ShaderPtr mVertexShader;
    NFE::Renderer::ShaderPtr mPixelShader;
    NFE::Renderer::BufferPtr mVertexBuffer;
    NFE::Renderer::VertexLayoutPtr mVertexLayout;
    NFE::Renderer::BufferPtr mIndexBuffer;
    NFE::Renderer::BufferPtr mConstantBuffer;

    NFE::Renderer::ResourceBindingSetPtr mVSBindingSet;
    NFE::Renderer::ResourceBindingLayoutPtr mResBindingLayout;
    NFE::Renderer::ResourceBindingInstancePtr mVSBindingInstance;

    int mCBufferSlot;

    // Used for objects rotation in Constant Buffer scenes and onward
    float mAngle;

    // how many instances will be drawn?
    int mGridSize;

    int mVSBindingSlot;

    // cbuffer mode
    NFE::Renderer::BufferMode mCBufferMode;

    NFE::Common::ThreadPool mThreadPool;

    // Releases only subscene-related resources. Backbuffer, RT and BlendState stay intact.
    void ReleaseSubsceneResources() override;

    // Resource creators for subscenes
    bool CreateCommandRecorders();
    bool CreateShaders(NFE::Renderer::BufferMode cbufferMode);
    bool CreateVertexBuffer();
    bool CreateIndexBuffer();
    bool CreateConstantBuffer(NFE::Renderer::BufferMode cbufferMode);

    // Subscenes
    bool CreateSubSceneEmpty();
    bool CreateSubSceneNormal(NFE::Renderer::BufferMode cbufferMode, int gridSize);

    // drawing task (will be called from thread pool)
    void DrawTask(const NFE::Common::TaskContext& ctx, int i, int j);

public:
    MultithreadedScene();
    ~MultithreadedScene();

    bool OnInit(void* winHandle) override;
    void Draw(float dt) override;
    void Release() override;
};
