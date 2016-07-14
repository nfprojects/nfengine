/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of TessellationScene functions
 */

#pragma once

#include "Scene.hpp"

class TessellationScene : public Scene
{
    // Renderer interfaces generated by BasicScene
    std::unique_ptr<NFE::Renderer::IBackbuffer> mWindowBackbuffer;
    std::unique_ptr<NFE::Renderer::IRenderTarget> mWindowRenderTarget;
    std::unique_ptr<NFE::Renderer::IPipelineState> mPipelineState;

    std::unique_ptr<NFE::Renderer::IShader> mVertexShader;
    std::unique_ptr<NFE::Renderer::IShader> mHullShader;
    std::unique_ptr<NFE::Renderer::IShader> mDomainShader;
    std::unique_ptr<NFE::Renderer::IShader> mPixelShader;

    std::unique_ptr<NFE::Renderer::IShaderProgram> mShaderProgram;
    std::unique_ptr<NFE::Renderer::IBuffer> mVertexBuffer;
    std::unique_ptr<NFE::Renderer::IVertexLayout> mVertexLayout;
    std::unique_ptr<NFE::Renderer::IResourceBindingLayout> mResBindingLayout;

    // Releases only subscene-related resources. Backbuffer, RT and BlendState stay intact.
    void ReleaseSubsceneResources();

    // Resource creators for subscenes
    bool CreateShaderProgram();
    bool CreateVertexBuffer();

    // Subscenes
    bool CreateSubSceneBezierLine();
    // TODO more subscenes testing triangle patches, variable tesselation factor, etc.

public:
    TessellationScene();
    ~TessellationScene();

    bool OnInit(void* winHandle) override;
    void Draw(float dt);
    void Release();
};
