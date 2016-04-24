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

// TODO this has a LOT of members. Consider organizing some of them together.
class CommandBuffer : public ICommandBuffer
{
    friend class Device;

    GLenum mCurrentIndexBufferFormat;
    GLenum mCurrentStencilFunc;
    GLuint mCurrentStencilRef;
    GLuint mCurrentStencilMask;
    RenderTarget* mCurrentRenderTarget;
    int mCurrentVertexLayoutElementsNum;

    std::vector<IBuffer*> mSetVertexBuffers;
    IBuffer* mSetIndexBuffer;
    IBuffer* mSetConstantBuffer;
    IVertexLayout* mSetVertexLayout;
    ITexture* mSetTexture;
    ISampler* mSetSampler;

    GLint mSetTextureSlot;
    GLint mSetSamplerSlot;
    GLuint mSetShaderProgram;
    GLint mSetConstantBufferSlot;

    bool mVertexBufferNeedsUpdate;
    bool mIndexBufferNeedsUpdate;
    bool mConstantBufferNeedsUpdate;
    bool mVertexLayoutNeedsUpdate;
    bool mTextureNeedsUpdate;
    bool mSamplerNeedsUpdate;

    // SSO-related variables
    bool mSSOEnabled;
    GLuint mProgramPipeline;

    /// Private methods which will bind just set resources
    void BindVertexBuffer();
    void BindVertexBufferAndLayout();
    void BindIndexBuffer();
    void BindConstantBuffer();
    void BindVertexLayout();
    void BindTexture();
    void BindSampler();

    void SetBlendState(const BlendStateDesc& desc);
    void SetRasterizerState(const RasterizerStateDesc& desc);
    void SetDepthState(const DepthStateDesc& desc);

public:
    CommandBuffer();
    ~CommandBuffer();

    /// Resources setup methods
    void Reset();
    void SetVertexBuffers(int num, IBuffer** vertexBuffers, int* strides, int* offsets);
    void SetIndexBuffer(IBuffer* indexBuffer, IndexBufferFormat format);
    void SetSamplers(ISampler** samplers, int num, ShaderType target, int slotOffset);
    void SetTextures(ITexture** textures, int num, ShaderType target, int slotOffset);
    void SetConstantBuffers(IBuffer** constantBuffers, int num, ShaderType target, int slotOffset);
    void SetRenderTarget(IRenderTarget* renderTarget);
    void SetShaderProgram(IShaderProgram* shaderProgram);
    void SetPipelineState(IPipelineState* state);
    void SetStencilRef(unsigned char ref);
    void SetViewport(float left, float width, float top, float height,
                     float minDepth, float maxDepth);
    void SetScissors(int left, int top, int right, int bottom);

    /// Executives

    void* MapBuffer(IBuffer* buffer, MapType type);
    void UnmapBuffer(IBuffer* buffer);
    bool WriteBuffer(IBuffer* buffer, size_t offset, size_t size, const void* data);
    bool ReadBuffer(IBuffer* buffer, size_t offset, size_t size, void* data);
    void CopyTexture(ITexture* src, ITexture* dest);
    bool ReadTexture(ITexture* tex, void* data);
    void Clear(int flags, const float* color, float depthValue, unsigned char stencilValue);
    void Draw(PrimitiveType type, int vertexNum, int instancesNum, int vertexOffset,
              int instanceOffset);
    void DrawIndexed(PrimitiveType type, int indexNum, int instancesNum, int indexOffset,
                     int vertexOffset, int instanceOffset);
    ICommandList* Finish();
    void Execute(ICommandList* commandList);

    /// Debugging
    void BeginDebugGroup(const char* text);
    void EndDebugGroup();
    void InsertDebugMarker(const char* text);
};

} // namespace Renderer
} // namespace NFE
