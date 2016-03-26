/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of VertexBuffersScene functions
 *
 * This test checks following functionalities:
 * * binding multiple vertex buffers (one containing positions and second containing color)
 * * instancing - binding third buffer containing per-instance data
 * * dynamic vertex buffers
 */

#pragma once

#include "Scene.hpp"

class VertexBuffersScene : public Scene
{
    // Renderer interfaces generated by VertexBuffersScene
    std::unique_ptr<NFE::Renderer::IBackbuffer> mWindowBackbuffer;
    std::unique_ptr<NFE::Renderer::IRenderTarget> mWindowRenderTarget;

    std::unique_ptr<NFE::Renderer::IShader> mVertexShader;
    std::unique_ptr<NFE::Renderer::IShader> mPixelShader;
    std::unique_ptr<NFE::Renderer::IShaderProgram> mShaderProgram;

    std::unique_ptr<NFE::Renderer::IBuffer> mPositionsVertexBuffer;
    std::unique_ptr<NFE::Renderer::IBuffer> mColorVertexBuffer;
    std::unique_ptr<NFE::Renderer::IBuffer> mInstanceBuffer;
    std::unique_ptr<NFE::Renderer::IBuffer> mIndexBuffer;
    std::unique_ptr<NFE::Renderer::IVertexLayout> mVertexLayout;

    std::unique_ptr<NFE::Renderer::IPipelineState> mPipelineState;

    // Releases only subscene-related resources. Backbuffer, RT and BlendState stay intact.
    void ReleaseSubsceneResources();

    // Resource creators for subscenes
    bool CreateShaderProgram(bool useInstancing);
    bool CreateBuffers(bool withInstanceBuffer);

    // Subscenes
    bool CreateSubSceneSimple();
    bool CreateSubSceneInstancing();

public:
    VertexBuffersScene();
    ~VertexBuffersScene();

    bool OnInit(void* winHandle);
    bool OnSwitchSubscene();
    void Draw(float dt);
    void Release();
};
