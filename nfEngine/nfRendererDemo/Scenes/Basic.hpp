/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Declarations of Scene functions
 *
 * The subscenes described in this file are meant to help in new Renderer backend creation process.
 * In theory, each subscene should add a small amount of Renderers Interfaces (ideally, only one),
 * which will represent specific Renderers features. The subscenes should rely on each other - more
 * complex subscene will add new Interface initialization to simplier scenes. This way it can be
 * easily tracked which features are yet to be implemented in a newly developed Renderer backend.
 */

#pragma once

#include "Scene.hpp"

class BasicScene: public Scene
{
    // Renderer interfaces generated by BasicScene
    std::unique_ptr<NFE::Renderer::IBackbuffer> mWindowBackbuffer;
    std::unique_ptr<NFE::Renderer::IRenderTarget> mWindowRenderTarget;
    std::unique_ptr<NFE::Renderer::IPipelineState> mPipelineState;
    std::unique_ptr<NFE::Renderer::IShader> mVertexShader;
    std::unique_ptr<NFE::Renderer::IShader> mPixelShader;
    std::unique_ptr<NFE::Renderer::IShaderProgram> mShaderProgram;
    std::unique_ptr<NFE::Renderer::IBuffer> mVertexBuffer;
    std::unique_ptr<NFE::Renderer::IVertexLayout> mVertexLayout;
    std::unique_ptr<NFE::Renderer::IBuffer> mIndexBuffer;
    std::unique_ptr<NFE::Renderer::IBuffer> mConstantBuffer;
    std::unique_ptr<NFE::Renderer::ISampler> mSampler;
    std::unique_ptr<NFE::Renderer::ITexture> mTexture;

    // Used for objects rotation in Constant Buffer scenes and onward
    float mAngle;

    // Releases only subscene-related resources. Backbuffer, RT and BlendState stay intact.
    void ReleaseSubsceneResources();

    // Resource creators for subscenes
    bool CreateShaderProgram(bool useCBuffer, bool useTexture);
    bool CreateVertexBuffer(bool withExtraVert);
    bool CreateIndexBuffer();
    bool CreateConstantBuffer();
    bool CreateTexture();

    // Subscenes
    bool CreateSubSceneEmpty();
    bool CreateSubSceneVertexBuffer();
    bool CreateSubSceneIndexBuffer();
    bool CreateSubSceneConstantBuffer();
    bool CreateSubSceneTexture();

public:
    BasicScene();
    ~BasicScene();

    bool OnInit(void* winHandle);
    void Draw(float dt);
    void Release();
};
