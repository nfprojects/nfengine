/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 11 render's command buffer.
 */

#pragma once

#include "../RendererInterface/CommandBuffer.hpp"
#include "PipelineState.hpp"
#include "Common.hpp"

namespace NFE {
namespace Renderer {

class RenderTarget;

class CommandBuffer : public ICommandBuffer
{
    friend class Device;

    unsigned char mStencilRef;
    unsigned char mCurrentStencilRef;
    PrimitiveType mCurrentPrimitiveType;
    RenderTarget* mCurrentRenderTarget;
    DepthState* mDepthState;
    DepthState* mCurrentDepthState;
    D3DPtr<ID3D11DeviceContext> mContext;
    D3DPtr<ID3DUserDefinedAnnotation> mUserDefinedAnnotation;

    ShaderProgramDesc mBoundShaders;

    void UpdateState(PrimitiveType primitiveType);

public:
    CommandBuffer(ID3D11DeviceContext* deviceContext);
    ~CommandBuffer();

    /// Shader resources setup methods

    void Reset();
    void SetVertexLayout(IVertexLayout* vertexLayout);
    void SetVertexBuffers(int num, IBuffer** vertexBuffers, int* strides, int* offsets);
    void SetIndexBuffer(IBuffer* indexBuffer, IndexBufferFormat format);
    void SetSamplers(ISampler** samplers, int num, ShaderType target);
    void SetTextures(ITexture** textures, int num, ShaderType target);
    void SetConstantBuffers(IBuffer** constantBuffers, int num, ShaderType target);
    void SetRenderTarget(IRenderTarget* renderTarget);
    void SetShaderProgram(IShaderProgram* shaderProgram);
    void SetShader(IShader* shader);
    void SetBlendState(IBlendState* state);
    void SetRasterizerState(IRasterizerState* state);
    void SetDepthState(IDepthState* state);
    void SetStencilRef(unsigned char ref);
    void SetViewport(float left, float width, float top, float height,
                     float minDepth, float maxDepth);

    /// "Executive" methods

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
