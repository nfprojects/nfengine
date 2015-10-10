/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of OpenGL 4 Command Buffer
 */

#pragma once

#include "../RendererInterface/CommandBuffer.hpp"

#include "RenderTarget.hpp"

namespace NFE {
namespace Renderer {

class CommandBuffer : public ICommandBuffer
{
    friend class Device;

    GLenum mCurrentIndexBufferFormat;
    RenderTarget* mCurrentRenderTarget;
    int mCurrentVertexLayoutElementsNum;

    IBuffer* mSetVertexBuffer;
    IBuffer* mSetIndexBuffer;
    IBuffer* mSetConstantBuffer;
    IVertexLayout* mSetVertexLayout;

    GLuint mSetShaderProgram;
    GLint mSetConstantBufferSlot;

    bool mVertexBufferNeedsUpdate;
    bool mIndexBufferNeedsUpdate;
    bool mConstantBufferNeedsUpdate;
    bool mVertexLayoutNeedsUpdate;

    // SSO-related variables
    bool mSSOEnabled;
    GLuint mProgramPipeline;
    GLuint mVAO;   //< For linux needs. This will probably be used for multiple VB support.

    /// Private methods which will bind just set resources
    void BindVertexBuffer();
    void BindIndexBuffer();
    void BindConstantBuffer();
    void BindVertexLayout();

public:
    CommandBuffer();
    ~CommandBuffer();

    /// Resources setup methods
    void Reset();
    void SetVertexLayout(IVertexLayout* vertexLayout);
    void SetVertexBuffers(int num, IBuffer** vertexBuffers, int* strides, int* offsets);
    void SetIndexBuffer(IBuffer* indexBuffer, IndexBufferFormat format);
    void SetSamplers(ISampler** samplers, int num, ShaderType target, int slotOffset);
    void SetTextures(ITexture** textures, int num, ShaderType target, int slotOffset);
    void SetConstantBuffers(IBuffer** constantBuffers, int num, ShaderType target, int slotOffset);
    void SetRenderTarget(IRenderTarget* renderTarget);
    void SetShaderProgram(IShaderProgram* shaderProgram);
    void SetShader(IShader* shader);
    void SetBlendState(IBlendState* state);
    void SetRasterizerState(IRasterizerState* state);
    void SetDepthState(IDepthState* state);
    void SetStencilRef(unsigned char ref);
    void SetViewport(float left, float width, float top, float height,
                     float minDepth, float maxDepth);

    /// Executives
    bool WriteBuffer(IBuffer* buffer, size_t offset, size_t size, const void* data);
    bool ReadBuffer(IBuffer* buffer, size_t offset, size_t size, void* data);
    void CopyTexture(ITexture* src, ITexture* dest);
    bool ReadTexture(ITexture* tex, void* data);
    void Clear(int flags, const float* color, float depthValue, unsigned char stencilValue);
    void Draw(PrimitiveType type, int vertexNum, int instancesNum, int vertexOffset,
              int instanceOffset);
    void DrawIndexed(PrimitiveType type, int indexNum, int instancesNum, int indexOffset,
                     int vertexOffset, int instanceOffset);
    void Execute(ICommandBuffer* commandBuffer, bool saveState);

    /// Debugging
    void BeginDebugGroup(const char* text);
    void EndDebugGroup();
    void InsertDebugMarker(const char* text);
};

} // namespace Renderer
} // namespace NFE
